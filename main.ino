#define ANIM_CLEAR 4
#define NEXT_PIECES 3
#define LEVEL_UP_ROWS 5
#define GAME_SPEED_FACTOR_NOM 3
#define GAME_SPEED_FACTOR_DENOM 1
#define SOFT_DROP_SPEED 8
#define DEBOUNCE_FRAMES 8
#define MAX_SAVES 63
#define SAVE_NAME_MAX_LENGTH 26

#define SCORE_SHIFT_R 2
#define SCORE_SHIFT_U 0

#define BOARD_SHIFT_R 2
#define BOARD_SHIFT_U 7

#define NEXT_SHIFT_R 23
#define NEXT_SHIFT_U 7

#define LEVEL_SHIFT_R 24
#define LEVEL_SHIFT_U 38

typedef struct {
    char name[SAVE_NAME_MAX_LENGTH];
    int16_t name_length;
    uint32_t score;
} save_t;

uint32_t save_count = 0;
save_t save_buffer[MAX_SAVES + 1];

save_t temp_save;

typedef enum {
    GAME_MENU,
    GAME_LEADERBOARD,
    GAME_START_ANIM,
    GAME_INGAME,
    GAME_INGAME_ROWCLEAR_ANIM,
    GAME_LOST_ANIM,
    GAME_LOST_ANIM_CLEAR,
    GAME_LOST_SAVESCORE,
} game_state;

static const int32_t gravity_table[] = {
    48, // Level 0
    43, // Level 1
    38, // Level 2
    33, // Level 3
    28, // Level 4
    23, // Level 5
    18, // Level 6
    13, // Level 7
    8,  // Level 8
    6,  // Level 9
    5,  // Level 10
    5,  // Level 11
    5,  // Level 12
    4,  // Level 13
    4,  // Level 14
    4,  // Level 15
    3,  // Level 16
    3,  // Level 17
    3,  // Level 18
    2,  // Level 19
    2,  // Level 20
    2,  // Level 21
    2,  // Level 22
    2,  // Level 23
    2,  // Level 24
    2,  // Level 25
    2,  // Level 26
    2,  // Level 27
    2,  // Level 28
    1   // Level 29+
};

#define RGB_SET(r, g, b) ((uint32_t)(b)) | ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16)
#define RGB_BLEND(a, b) ((((a) & 0x00FEFEFE) + ((b) & 0x00FEFEFE)) >> 1)  
#define RGB_INVERT(a) (((a) ^ 0x00FFFFFF) & 0x00FFFFFF)
#define RGB_MUL_COMP(a, val) ({ \
    uint32_t _a = (a); \
    uint32_t _val = (val); \
    uint32_t r = (((_a >> 8) & 0xFF) * _val) / 255; \
    uint32_t g = (((_a >> 16) & 0xFF) * _val) / 255; \
    uint32_t b = ((_a & 0xFF) * _val) / 255; \
    RGB_SET(r, g, b); \
})
#define RGB_COLOR_MOD(a, rm, gm, bm) ({ \
    uint32_t _a = (a); \
    uint32_t r = (((_a >> 8) & 0xFF) * (rm)) / 255; \
    uint32_t g = (((_a >> 16) & 0xFF) * (gm)) / 255; \
    uint32_t b = ((_a & 0xFF) * (bm)) / 255; \
    RGB_SET(r, g, b); \
})

#define SCORE_COLOR RGB_SET(64, 0, 0)
#define GAME_BORDER_COLOR RGB_SET(32, 0, 32)
#define NEXT_BORDER_COLOR RGB_SET(16, 0, 16)
#define PIECE_FALL_HIGHLIGHT_COLOR RGB_SET(16, 16, 16)
#define CLEAR_COLOR RGB_SET(0, 0, 0)

// Colors for each tetromino
const static uint32_t colors[8] =
    {
        RGB_SET(0, 255, 255), // I - Cyan
        RGB_SET(255, 255, 0), // O - Yellow
        RGB_SET(255, 0, 255), // T - Magenta
        RGB_SET(0, 255, 0),   // S - Green
        RGB_SET(255, 0, 0),   // Z - Red
        RGB_SET(0, 0, 255),   // J - Blue
        RGB_SET(255, 165, 0)  // L - Orange
};

const static uint16_t piece_mask[7][4] = {{0x000F, 0x1111, 0x000F, 0x1111}, {0x0033, 0x0033, 0x0033, 0x0033}, {0x0027, 0x0232, 0x0072, 0x0131}, {0x0036, 0x0231, 0x0036, 0x0231}, {0x0063, 0x0132, 0x0063, 0x0132}, {0x0047, 0x0322, 0x0071, 0x0113}, {0x0017, 0x0223, 0x0074, 0x0311}};
const static int piece_max_x[7][4] = {{10 - 4, 10 - 1, 10 - 4, 10 - 1}, {10 - 2, 10 - 2, 10 - 2, 10 - 2}, {10 - 3, 10 - 2, 10 - 3, 10 - 2}, {10 - 3, 10 - 2, 10 - 3, 10 - 2}, {10 - 3, 10 - 2, 10 - 3, 10 - 2}, {10 - 3, 10 - 2, 10 - 3, 10 - 2}, {10 - 3, 10 - 2, 10 - 3, 10 - 2}};
const static int piece_max_y[7][4] = {{20 - 1, 20 - 4, 20 - 1, 20 - 4}, {20 - 2, 20 - 2, 20 - 2, 20 - 2}, {20 - 2, 20 - 3, 20 - 2, 20 - 3}, {20 - 2, 20 - 3, 20 - 2, 20 - 3}, {20 - 2, 20 - 3, 20 - 2, 20 - 3}, {20 - 2, 20 - 3, 20 - 2, 20 - 3}, {20 - 2, 20 - 3, 20 - 2, 20 - 3}};
const static int piece_rot_x_shift[7][4] = {{0, 2, 0, 2}, {1, 1, 1, 1}, {0, 0, 0, 1}, {0, 0, 0, 0}, {0, 1, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}};
const static int piece_rot_y_shift[7][4] = {{1, 0, 1, 0}, {1, 1, 1, 1}, {1, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 1, 0}};
const static uint16_t charset[] = {0x7B6F, 0x4974, 0x73E7, 0x79E7, 0x49ED, 0x79CF, 0x7BCF, 0x4927, 0x7BEF, 0x79EF, 0x5BEA, 0x3AEB, 0x624E, 0x3B6B, 0x73CF, 0x13CF, 0x6B4E, 0x5BED, 0x7497, 0x7B24, 0x5AED, 0x7249, 0x5B7D, 0x1BEC, 0x7B6F, 0x13EF, 0x6F6A, 0x5AEF, 0x39CE, 0x2497, 0x7B6D, 0x2B6D, 0x5F6D, 0x5AAD, 0x24AD, 0x72A7, 0x7A00, 0x2000, 0x2400, 0x44D4, 0x01C0};

static uint32_t color_board[32];
static uint32_t color_board_anim[32];
static uint16_t board[32];
static uint16_t board_anim[32];
int current_piece;
int current_x;
static int current_y;
int current_rotation;
int next_piece[NEXT_PIECES];
int score;
int level;
int rows_cleared;
int anim_frames;
int frame;
int selector_x;
int selector_y;
int frames_till_fall;
game_state cur_state;

bool is_soft_dropping;
bool is_hard_dropping;
bool moved_on_last_iteration;

// Button instances
uint32_t button_up = 0;
uint32_t button_up_state = 0;
uint32_t button_down = 0;
uint32_t button_down_state = 0;
uint32_t button_right = 0;
uint32_t button_right_state = 0;
uint32_t button_left = 0;
uint32_t button_left_state = 0;
uint32_t button_special = 0;
uint32_t button_special_state = 0;

static uint32_t matrix_array[1536] = {0};

static uint32_t rng_seed = 0xdeadbeef;

uint32_t mulberry_rand()
{
    uint32_t z = (rng_seed += 0x6D2B79F5UL);
    z = (z ^ (z >> 15)) * (z | 1UL);
    z ^= z + (z ^ (z >> 7)) * (z | 61UL);
    return z ^ (z >> 14);
}

void update_display()
{
    for (int i = 0; i < 256; ++i)
    {
        int pixel_base_right = ((((15 - (i >> 4)) * 2 + 1) << 4) + ((((i >> 4) & 1) == 0) ? (i & 15) : (15 - (i & 15))));
        int pixel_base_left = (((i >> 4) << 5) + ((((i >> 4) & 1) == 0) ? (15 - (i & 15)) : (i & 15)));

        uint32_t var1 = matrix_array[pixel_base_right];
        matrix_array[pixel_base_right] = 0;
        uint32_t var2 = matrix_array[pixel_base_left];
        matrix_array[pixel_base_left] = 0;
        uint32_t var3 = matrix_array[pixel_base_right + 512];
        matrix_array[pixel_base_right + 512] = 0;
        uint32_t var4 = matrix_array[pixel_base_left + 512];
        matrix_array[pixel_base_left + 512] = 0;
        uint32_t var5 = matrix_array[pixel_base_right + 1024];
        matrix_array[pixel_base_right + 1024] = 0;
        uint32_t var6 = matrix_array[pixel_base_left + 1024];
        matrix_array[pixel_base_left + 1024] = 0;

        for (int u = 0; u < 24; ++u)
        {
            GPIOA->ODR = ((1 << 9) | (1 << 1) | (1 << 4));

            asm volatile("push {r0-r3}\npush {r0-r3}\npush {r0-r3}\n");
            asm("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
            asm volatile("pop {r0-r3}\npop {r0-r3}\npop {r0-r3}\n");

            GPIOA->ODR = ((var1 & 8388608) >> (23 - 9)) | ((var2 & 8388608) >> (23 - 4)) | ((var3 & 8388608) >> (23 - 1));

            asm volatile("push {r0-r3}\npush {r0-r3}\npush {r0-r3}\n");
            asm("nop\nnop\nnop\n");
            asm volatile("pop {r0-r3}\npop {r0-r3}\npop {r0-r3}\n");

            GPIOA->ODR = ((1 << 7) | (1 << 6) | (1 << 5));

            asm volatile("push {r0-r3}\npush {r0-r3}\npush {r0-r3}\n");
            asm("nop\nnop\nnop\nnop\nnop\n");
            asm volatile("pop {r0-r3}\npop {r0-r3}\npop {r0-r3}\n");

            GPIOA->ODR = ((var4 & 8388608) >> (23 - 7)) | ((var5 & 8388608) >> (23 - 6)) | ((var6 & 8388608) >> (23 - 5));

            asm volatile("push {r0-r3}\npush {r0-r3}\n");
            asm("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
            asm volatile("pop {r0-r3}\npop {r0-r3}\n");

            var1 <<= 1;
            var2 <<= 1;
            var3 <<= 1;
            var4 <<= 1;
            var5 <<= 1;
            var6 <<= 1;
        }

        GPIOA->ODR = 0;
    }
}

void render_tetromino(int base, uint16_t piece_mask, uint32_t color)
{
    if (piece_mask & 1)
    {
        matrix_array[base + 0 + 0] = color;
        matrix_array[base + 1 + 0] = color;
        matrix_array[base + 0 + 32] = color;
        matrix_array[base + 1 + 32] = color;
    }

    if (piece_mask & 2)
    {
        matrix_array[base + 2 + 0] = color;
        matrix_array[base + 3 + 0] = color;
        matrix_array[base + 2 + 32] = color;
        matrix_array[base + 3 + 32] = color;
    }

    if (piece_mask & 4)
    {
        matrix_array[base + 4 + 0] = color;
        matrix_array[base + 5 + 0] = color;
        matrix_array[base + 4 + 32] = color;
        matrix_array[base + 5 + 32] = color;
    }

    if (piece_mask & 8)
    {
        matrix_array[base + 6 + 0] = color;
        matrix_array[base + 7 + 0] = color;
        matrix_array[base + 6 + 32] = color;
        matrix_array[base + 7 + 32] = color;
    }

    if (piece_mask & 16)
    {
        matrix_array[base + 0 + 0 + 64] = color;
        matrix_array[base + 1 + 0 + 64] = color;
        matrix_array[base + 0 + 32 + 64] = color;
        matrix_array[base + 1 + 32 + 64] = color;
    }

    if (piece_mask & 32)
    {
        matrix_array[base + 2 + 0 + 64] = color;
        matrix_array[base + 3 + 0 + 64] = color;
        matrix_array[base + 2 + 32 + 64] = color;
        matrix_array[base + 3 + 32 + 64] = color;
    }

    if (piece_mask & 64)
    {
        matrix_array[base + 4 + 0 + 64] = color;
        matrix_array[base + 5 + 0 + 64] = color;
        matrix_array[base + 4 + 32 + 64] = color;
        matrix_array[base + 5 + 32 + 64] = color;
    }

    if (piece_mask & 256)
    {
        matrix_array[base + 0 + 0 + 64 + 64] = color;
        matrix_array[base + 1 + 0 + 64 + 64] = color;
        matrix_array[base + 0 + 32 + 64 + 64] = color;
        matrix_array[base + 1 + 32 + 64 + 64] = color;
    }

    if (piece_mask & 512)
    {
        matrix_array[base + 2 + 0 + 64 + 64] = color;
        matrix_array[base + 3 + 0 + 64 + 64] = color;
        matrix_array[base + 2 + 32 + 64 + 64] = color;
        matrix_array[base + 3 + 32 + 64 + 64] = color;
    }

    if (piece_mask & 4096)
    {
        matrix_array[base + 0 + 0 + 64 + 64 + 64] = color;
        matrix_array[base + 1 + 0 + 64 + 64 + 64] = color;
        matrix_array[base + 0 + 32 + 64 + 64 + 64] = color;
        matrix_array[base + 1 + 32 + 64 + 64 + 64] = color;
    }
}

void render_char(int base, uint16_t cur_digit_mask, uint32_t color)
{
    if (cur_digit_mask & 1)
    {
        matrix_array[base + 0] = color;
    }

    if (cur_digit_mask & 2)
    {
        matrix_array[base + 1] = color;
    }

    if (cur_digit_mask & 4)
    {
        matrix_array[base + 2] = color;
    }

    if (cur_digit_mask & 8)
    {
        matrix_array[base + 0 + 32] = color;
    }

    if (cur_digit_mask & 16)
    {
        matrix_array[base + 1 + 32] = color;
    }

    if (cur_digit_mask & 32)
    {
        matrix_array[base + 2 + 32] = color;
    }

    if (cur_digit_mask & 64)
    {
        matrix_array[base + 0 + 32 + 32] = color;
    }

    if (cur_digit_mask & 128)
    {
        matrix_array[base + 1 + 32 + 32] = color;
    }

    if (cur_digit_mask & 256)
    {
        matrix_array[base + 2 + 32 + 32] = color;
    }

    if (cur_digit_mask & 512)
    {
        matrix_array[base + 0 + 32 + 32 + 32] = color;
    }

    if (cur_digit_mask & 1024)
    {
        matrix_array[base + 1 + 32 + 32 + 32] = color;
    }

    if (cur_digit_mask & 2048)
    {
        matrix_array[base + 2 + 32 + 32 + 32] = color;
    }

    if (cur_digit_mask & 4096)
    {
        matrix_array[base + 0 + 32 + 32 + 32 + 32] = color;
    }

    if (cur_digit_mask & 8192)
    {
        matrix_array[base + 1 + 32 + 32 + 32 + 32] = color;
    }

    if (cur_digit_mask & 16384)
    {
        matrix_array[base + 2 + 32 + 32 + 32 + 32] = color;
    }
}

void render_frame(int base, int width, int height, uint32_t color)
{
    for (int i = 0; i < width; ++i)
    {
        matrix_array[base + i] = color;                     // Top edge
        matrix_array[base + (height - 1) * 32 + i] = color; // Bottom edge
    }

    for (int i = 0; i < height; ++i)
    {
        matrix_array[base + i * 32] = color;               // Left edge
        matrix_array[base + i * 32 + (width - 1)] = color; // Right edge
    }
}

void render_animated_frame(int base, int width, int height, uint32_t color, int frame)
{
    int perimeter = 2 * (width + height) - 4; // Total edge pixels (excluding corner overlaps)
    
    for (int p = 0; p < perimeter; ++p)
    {
        // Create a moving pattern that wraps around the entire perimeter
        bool is_lit = ((p + frame) % 6) < 4;
        
        if (!is_lit)
            continue;
        
        int x, y;
        
        // Map perimeter position to actual coordinates
        if (p < width)
        {
            // Top edge
            x = p;
            y = 0;
        }
        else if (p < width + height - 1)
        {
            // Right edge
            x = width - 1;
            y = p - width + 1;
        }
        else if (p < 2 * width + height - 2)
        {
            // Bottom edge (right to left)
            x = width - 1 - (p - width - height + 2);
            y = height - 1;
        }
        else
        {
            // Left edge (bottom to top)
            x = 0;
            y = height - 1 - (p - 2 * width - height + 3);
        }
        
        matrix_array[base + y * 32 + x] = color;
    }
}

void render_string(int base, char *str, uint32_t color)
{
    while (*str)
    {
        char cur_char = *str;

        if (cur_char == ' ')
        {
            ++str;
            base += 4;
            continue;
        }

        cur_char = (cur_char >= 'a' && cur_char <= 'z') ? (cur_char - 'a' + 'A') : cur_char;
        cur_char = (cur_char >= 'A' && cur_char <= 'Z') ? (cur_char - ('A' - 10)) : cur_char;
        cur_char = (cur_char >= '0' && cur_char <= '9') ? (cur_char - '0') : cur_char;

        render_char(base, charset[cur_char], color);

        base += 4;

        ++str;
    }
}

void render_sliding_text(int base, char *str, int max_width, uint32_t offset, uint32_t color)
{
    int char_width = 4;
    int glyph_width = 3;
    size_t str_len = strlen(str);
    if (str_len == 0)
        return;

    int total_width = (int)str_len * char_width;
    offset %= total_width;

    int sub_offset = offset % char_width;
    int skip_chars = offset / char_width;

    int current_index = skip_chars % str_len;
    int current_pos = base - sub_offset;

    while (current_pos < base + max_width)
    {
        char cur_char = str[current_index];

        if (cur_char != ' ')
        {
            cur_char = (cur_char >= 'a' && cur_char <= 'z') ? (cur_char - 'a' + 'A') : cur_char;
            cur_char = (cur_char >= 'A' && cur_char <= 'Z') ? (cur_char - ('A' - 10)) : cur_char;
            cur_char = (cur_char >= '0' && cur_char <= '9') ? (cur_char - '0') : cur_char;

            uint32_t mask = charset[cur_char];

            int left_clip = (base > current_pos) ? (base - current_pos) : 0;
            int right_clip = (current_pos + glyph_width > base + max_width) ? (current_pos + glyph_width - (base + max_width)) : 0;

            int draw_width = glyph_width - left_clip - right_clip;
            if (draw_width > 0)
            {
                uint32_t new_mask = 0;
                for (int row = 0; row < 5; row++)
                {
                    uint32_t row_bits = (mask >> (row * 3)) & 0x7;
                    uint32_t clipped = (row_bits >> left_clip) & ((1U << draw_width) - 1);
                    new_mask |= clipped << (row * 3);
                }
                int draw_pos = current_pos + left_clip;
                render_char(draw_pos, new_mask, color);
            }
        }

        current_pos += char_width;
        current_index = (current_index + 1) % str_len;
    }
}

int main()
{
    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS; // Set Flash latency, prefetch, and caches

    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY))
    {
    } // Enable HSE and wait for ready

    // Enable PWR clock and set voltage scaling to scale 1 (for 180 MHz)
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS; // VOS = 3 (scale 1, high performance)

    // Enable overdrive mode for 180 MHz
    PWR->CR |= PWR_CR_ODEN;
    while (!(PWR->CSR & PWR_CSR_ODRDY))
    {
    }
    PWR->CR |= PWR_CR_ODSWEN;
    while (!(PWR->CSR & PWR_CSR_ODSWRDY))
    {
    }

    // Configure prescalers (AHB=1, APB1=4, APB2=2)
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 |  // AHB = SYSCLK / 1 = 180 MHz
                 RCC_CFGR_PPRE1_DIV4 | // APB1 = AHB / 4 = 45 MHz (max 45 MHz)
                 RCC_CFGR_PPRE2_DIV2;  // APB2 = AHB / 2 = 90 MHz (max 90 MHz)

    // Configure main PLL (HSE / 4 * 180 / 2 = 180 MHz)
    RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE |          // Source = HSE
                   (4UL << RCC_PLLCFGR_PLLM_Pos) |   // PLLM = 4 (8 MHz / 4 = 2 MHz input)
                   (180UL << RCC_PLLCFGR_PLLN_Pos) | // PLLN = 180 (2 MHz * 180 = 360 MHz VCO)
                   (0UL << RCC_PLLCFGR_PLLP_Pos) |   // PLLP = 2 (360 / 2 = 180 MHz SYSCLK)
                   (4UL << RCC_PLLCFGR_PLLQ_Pos);    // PLLQ = 4 (360 / 4 = 90 MHz, not used here)

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY))
    {
    } // Enable PLL and wait for ready

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
    {
    } // Switch system clock to PLL and wait

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable GPIOA
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // enable GPIOB

    GPIOA->MODER |= (GPIO_MODER_MODER5_0) | (GPIO_MODER_MODER6_0) | (GPIO_MODER_MODER7_0) | (GPIO_MODER_MODER1_0) | (GPIO_MODER_MODER4_0) | (GPIO_MODER_MODER9_0) | (GPIO_MODER_MODER0_0);
    GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED5) | (GPIO_OSPEEDR_OSPEED6) | (GPIO_OSPEEDR_OSPEED7) | (GPIO_OSPEEDR_OSPEED1) | (GPIO_OSPEEDR_OSPEED4) | (GPIO_OSPEEDR_OSPEED9) | (GPIO_OSPEEDR_OSPEED0);

__reset:

    for (int i = 0; i < NEXT_PIECES; ++i)
        next_piece[i] = mulberry_rand() % (uint32_t)7;
    current_piece = mulberry_rand() % (uint32_t)7;
    for (int i = 0; i < 24; ++i)
        board[i] = 0;
    for (int i = 0; i < 24; ++i)
        color_board[i] = 0;
    current_x = piece_max_x[current_piece][0] >> 1;
    current_y = 0;
    current_rotation = 0;
    score = 0;
    level = 0;
    rows_cleared = 0;
    is_soft_dropping = false;
    is_hard_dropping = false;
    moved_on_last_iteration = true;
    anim_frames = 0;
    selector_x = 0;
    selector_y = 0;
    frame = 0;
    frames_till_fall = (gravity_table[((level > 29) ? 29 : level)] * (GAME_SPEED_FACTOR_NOM)) / (GAME_SPEED_FACTOR_DENOM);
    frames_till_fall = (frames_till_fall > 0) ? frames_till_fall : 1;
    cur_state = GAME_MENU;

    char temp_buf[128];

    while (true)
    {
        if (cur_state == GAME_LEADERBOARD)
        {
            render_sliding_text(2 * 32 + 2, "leaderboard  ", 28, frame >> 5, RGB_SET(255, 255, 255));
            render_animated_frame(0, 32, 9, RGB_SET(64, 0, 64), frame >> 4);

            // Pre-pass: Find the maximum total length for alignment
            int max_total_len = 0;
            for (int i = 0; i < 4; ++i)
            {
                int id = selector_y * 4 + i + 1;
                
                if (id > 99 || id - 1 >= save_count)
                    continue;
                
                // Count score digits
                int score = save_buffer[id - 1].score;
                int temp_score = score;
                int score_len = 0;
                do {
                    score_len++;
                    temp_score /= 10;
                } while (temp_score > 0);
                
                int name_len = save_buffer[id - 1].name_length;
                int total_len = score_len + 1 + name_len + 3;
                
                if (total_len > max_total_len)
                    max_total_len = total_len;
            }

            // Main rendering loop
            for (int i = 0; i < 4; ++i)
            {
                int id = selector_y * 4 + i + 1;

                if (id > 99)
                    continue;

                int temp = id;
                render_char((11 + i * 7) * 32 + 5, charset[temp % 10], RGB_SET(0, 255, 0));
                temp /= 10;
                render_char((11 + i * 7) * 32 + 1, charset[temp % 10], RGB_SET(0, 255, 0));

                if (id - 1 < save_count)
                {
                    int score = save_buffer[id - 1].score;
                    int temp_score = score;
                    int score_len = 0;

                    // Count digits
                    do {
                        score_len++;
                        temp_score /= 10;
                    } while (temp_score > 0);

                    // Write digits in reverse
                    for (int j = score_len - 1; j >= 0; j--) {
                        temp_buf[j] = '0' + (score % 10);
                        score /= 10;
                    }

                    int name_len = save_buffer[id - 1].name_length;

                    // Fill rest with spaces up to max_total_len
                    for (int j = score_len; j < max_total_len; j++) {
                        temp_buf[j] = ' ';
                    }
                    temp_buf[max_total_len] = '\0';

                    // Render score
                    render_sliding_text((11 + i * 7) * 32 + 9, temp_buf, 22, frame >> 5, RGB_SET(255, 0, 0));

                    for (int j = 0; j < score_len; j++) {
                        temp_buf[j] = ' ';
                    }

                    // Copy name after the space where score was
                    memcpy(temp_buf + score_len + 1, save_buffer[id - 1].name, name_len);
                    
                    // Fill remaining space to match max length
                    for (int j = score_len + 1 + name_len; j < max_total_len; j++) {
                        temp_buf[j] = ' ';
                    }
                    temp_buf[max_total_len] = '\0';

                    // Render name
                    render_sliding_text((11 + i * 7) * 32 + 9, temp_buf, 22, frame >> 5, RGB_SET(255, 255, 255));
                }
                else
                {
                    render_string((11 + i * 7) * 32 + 9, "\x28\x28\x28\x28\x28\x28", RGB_SET(255, 255, 255));
                }
            }

            int base = 41 * 32 + 12;

            render_animated_frame(base - 66, 9, 9, RGB_MUL_COMP(RGB_SET(64, 0, 64), (selector_x == 1) ? 255 : 128), frame >> 4);

            matrix_array[base + 2] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 1] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 2] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 3] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 32 + 0] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 32 + 2] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 32 + 4] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 32 + 32 + 2] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 32 + 32 + 32 + 2] = RGB_SET(255, 255, 255);

            if (selector_x == 1)
            {
                base -= 33;
                for (int i = 0; i < 7; ++i)
                {
                    matrix_array[i + base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base]), 0, 255, 0);
                    matrix_array[i + base + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                }
            }

            base = 41 * 32 + 22;

            render_animated_frame(base - 66, 9, 9, RGB_MUL_COMP(RGB_SET(64, 0, 64), (selector_x == 2) ? 255 : 128), frame >> 4);

            matrix_array[base + 0] = RGB_SET(255, 255, 255);
            matrix_array[base + 4] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 1] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 3] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 32 + 2] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 32 + 32 + 1] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 32 + 32 + 3] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 32 + 32 + 32 + 0] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 32 + 32 + 32 + 4] = RGB_SET(255, 255, 255);

            if (selector_x == 2)
            {
                base -= 33;
                for (int i = 0; i < 7; ++i)
                {
                    matrix_array[i + base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base]), 0, 255, 0);
                    matrix_array[i + base + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                }
            }

            base = 41 * 32 + 2;

            render_animated_frame(base - 66, 9, 9, RGB_MUL_COMP(RGB_SET(64, 0, 64), (selector_x == 0) ? 255 : 128), frame >> 4);

            matrix_array[base + 32 + 32 + 32 + 32 + 2] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 32 + 32 + 1] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 32 + 32 + 2] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 32 + 32 + 3] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 32 + 0] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 32 + 2] = RGB_SET(255, 255, 255);
            matrix_array[base + 32 + 32 + 4] = RGB_SET(255, 255, 255);

            matrix_array[base + 32 + 2] = RGB_SET(255, 255, 255);

            matrix_array[base + 2] = RGB_SET(255, 255, 255);

            if (selector_x == 0)
            {
                base -= 33;
                for (int i = 0; i < 7; ++i)
                {
                    matrix_array[i + base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base]), 0, 255, 0);
                    matrix_array[i + base + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[i + base + 32 + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[i + base + 32 + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                }
            }

            for (volatile int i = 0; i < 1024; ++i)
            ; // we are too fast!

            update_display();

            button_left = (button_left > 0) ? (button_left - 1) : button_left;
            button_up = (button_up > 0) ? (button_up - 1) : button_up;
            button_down = (button_down > 0) ? (button_down - 1) : button_down;
            button_special = (button_special > 0) ? (button_special - 1) : button_special;
            button_right = (button_right > 0) ? (button_right - 1) : button_right;

            uint32_t temp_state;

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_6);

            if (temp_state != button_left_state)
            {
                button_left_state = temp_state;
                if (button_left == 0 && button_left_state != 0)
                {
                    selector_x--;
                    selector_x = (selector_x < 0) ? 0 : selector_x;
                }
                button_left = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_3);

            if (temp_state != button_right_state)
            {
                button_right_state = temp_state;
                if (button_right == 0 && button_right_state != 0)
                {
                    selector_x++;
                    selector_x = (selector_x > 2) ? 2 : selector_x;
                }
                button_right = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOA->IDR & GPIO_IDR_IDR_10);

            if (temp_state != button_special_state)
            {
                button_special_state = temp_state;
                if (button_special == 0 && button_special_state != 0)
                {
                    if (selector_x == 0)
                    {
                        selector_y++;
                        selector_y = (selector_y > 24) ? 24 : selector_y;
                    }
                    else if (selector_x == 1)
                    {
                        selector_y--;
                        selector_y = (selector_y < 0) ? 0 : selector_y;
                    }
                    else
                    {
                        selector_y = 0;
                        selector_x = 0;
                        cur_state = GAME_MENU;
                        frame = 0;
                        continue;
                    }
                }
                button_special = DEBOUNCE_FRAMES;
            }
            
            ++frame;
        }
        else if (cur_state == GAME_MENU) 
        {
            --frames_till_fall;

            if (frames_till_fall == 0)
            {
                frames_till_fall = 20;

                uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

                if (((((board[current_y + 1] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 4] >> current_x) & ((cur_piece_mask >> 12)))) != 0) || current_y == piece_max_y[current_piece][current_rotation])
                {
                    board[current_y + 0] |= (((cur_piece_mask) & 0xF) << current_x);
                    board[current_y + 1] |= (((cur_piece_mask >> 4) & 0xF) << current_x);
                    board[current_y + 2] |= (((cur_piece_mask >> 8) & 0xF) << current_x);
                    board[current_y + 3] |= ((cur_piece_mask >> 12) << current_x);

                    if (cur_piece_mask & 1)
                        color_board[current_y + 0] |= (current_piece << (current_x * 3 + 0));
                    if (cur_piece_mask & 2)
                        color_board[current_y + 0] |= (current_piece << (current_x * 3 + 3));
                    if (cur_piece_mask & 4)
                        color_board[current_y + 0] |= (current_piece << (current_x * 3 + 6));
                    if (cur_piece_mask & 8)
                        color_board[current_y + 0] |= (current_piece << (current_x * 3 + 9));

                    if (cur_piece_mask & 16)
                        color_board[current_y + 1] |= (current_piece << (current_x * 3 + 0));
                    if (cur_piece_mask & 32)
                        color_board[current_y + 1] |= (current_piece << (current_x * 3 + 3));
                    if (cur_piece_mask & 64)
                        color_board[current_y + 1] |= (current_piece << (current_x * 3 + 6));

                    if (cur_piece_mask & 256)
                        color_board[current_y + 2] |= (current_piece << (current_x * 3 + 0));
                    if (cur_piece_mask & 512)
                        color_board[current_y + 2] |= (current_piece << (current_x * 3 + 3));

                    if (cur_piece_mask & 4096)
                        color_board[current_y + 3] |= (current_piece << (current_x * 3 + 0));
                    
                    int rc = 0;    

                    for (int k = 0; k < 20; ++k)
                    {
                        if (board[k] == ((1 << 10) - 1))
                            ++rc;
                    }

                    if (rc)
                    {
                        int l = 20 - 1;
                        int k = 20 - 1;

                        for (; k >= 0; k--)
                        {
                            if (board[k] != ((1 << 10) - 1))
                            {
                                board[l] = board[k];
                                color_board[l] = color_board[k];
                                --l;
                            }
                        }

                        for (; l >= 0; l--)
                        {
                            board[l] = 0;
                            color_board[k] = 0;
                        }
                    }

                    switch (rc)
                    {
                    case 1:
                        score += 40 * (level + 1);
                        rows_cleared += 1;
                        break;
                    case 2:
                        score += 100 * (level + 1);
                        rows_cleared += 2;
                        break;
                    case 3:
                        score += 300 * (level + 1);
                        rows_cleared += 3;
                        break;
                    case 4:
                        score += 1200 * (level + 1);
                        rows_cleared += 4;
                        break;
                    default:
                        break;
                    }
                    
                    if (rows_cleared >= LEVEL_UP_ROWS)
                    {
                        rows_cleared -= LEVEL_UP_ROWS;
                        ++level;
                    }

                    current_piece = next_piece[0];
                    current_x = piece_max_x[current_piece][0] / 2;
                    current_y = 0;
                    current_rotation = 0;

                    for (int i = 0; i < NEXT_PIECES - 1; ++i)
                        next_piece[i] = next_piece[i + 1];

                    next_piece[NEXT_PIECES - 1] = mulberry_rand() % (uint32_t)7;

                    cur_piece_mask = piece_mask[current_piece][0];

                    if (((((board[current_y] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0)) // cant fit new tetromino
                    {
                        for (int i = 0; i < 24; ++i)
                            board[i] = 0;
                        for (int i = 0; i < 24; ++i)
                            color_board[i] = 0;
                    }
                }
                else 
                {
                    ++current_y;
                    ++score;
                }
            }

            render_frame(BOARD_SHIFT_R - 1 + (BOARD_SHIFT_U - 1) * 32, 22, 42, GAME_BORDER_COLOR);

            int temp = score;
            int base = SCORE_SHIFT_R + SCORE_SHIFT_U * 32 + 6 * 4;

            for (int i = 0; i < 7; ++i, temp /= 10)
            {
                render_char(base, charset[temp % 10], SCORE_COLOR);

                base -= 4;
            }

            base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32;

            for (int y = 0; y < 20; ++y, base += (32 + (32 - 20)))
            {
                uint32_t color_row = color_board[y];
                uint16_t board_row = board[y];

                for (int x = 0; x < 10; ++x, base += 2)
                {
                    uint32_t cur_color = (board_row & 1) ? (colors[color_row & 7]) : (CLEAR_COLOR);

                    matrix_array[base + 0 + 0] = cur_color;
                    matrix_array[base + 1 + 0] = cur_color;
                    matrix_array[base + 0 + 32] = cur_color;
                    matrix_array[base + 1 + 32] = cur_color;

                    color_row >>= 3;
                    board_row >>= 1;
                }
            }

            uint32_t color = colors[current_piece];
            uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

            for (int current_y_copy = current_y;; ++current_y_copy)
            {
                if (((((board[current_y_copy] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y_copy + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y_copy + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y_copy + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0) || current_y_copy - 1 == piece_max_y[current_piece][current_rotation]) // cant fit new tetromino
                {
                    break;
                }

                render_tetromino(BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + current_y_copy * 32 * 2 + current_x * 2, cur_piece_mask, PIECE_FALL_HIGHLIGHT_COLOR);
            }

            render_tetromino(BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + current_y * 32 * 2 + current_x * 2, cur_piece_mask, color);

            for(int i = 0; i < NEXT_PIECES; ++i)
            {
                render_tetromino(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 + ((next_piece[i] != 0) ? 1 : 0) + ((next_piece[i] == 1) ? 1 : 0) + ((next_piece[i] != 0) ? (2 * 32) : (3 * 32)), piece_mask[next_piece[i]][0], colors[next_piece[i]]);
                render_frame(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 - 33, 10, 10, NEXT_BORDER_COLOR);
            }

            temp = level;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32 + 4, charset[temp % 10], SCORE_COLOR);
            temp /= 10;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32, charset[temp % 10], SCORE_COLOR);

            for (int i = 0; i < 1536; ++i)
                matrix_array[i] = RGB_MUL_COMP(matrix_array[i], 32);

            for (volatile int i = 0; i < 1024; ++i)
            ; // we are too fast!

            //render_string(2 * 32 + 4, "tetris", RGB_SET(255, 255, 255));
            //render_animated_frame(1 * 32 + 3, 25, 7, RGB_SET(64, 0, 64), frame >> 4);

            render_string(14 * 32 + 6, "Start", RGB_SET(255, 255, 255));
            render_animated_frame(12 * 32 + 4, 23, 9, RGB_SET(64, 0, 64), frame >> 4);

            // render_string(24 * 32 + 4, "1 vs 1", RGB_SET(255, 255, 255));
            // render_animated_frame(22 * 32 + 2, 27, 9, RGB_SET(64, 0, 64), frame >> 4);

            render_sliding_text(34 * 32 + 2, "leaderboard  ", 28, frame >> 5, RGB_SET(255, 255, 255));
            render_animated_frame(32 * 32, 32, 9, RGB_SET(64, 0, 64), frame >> 4);

            if (selector_y == 0)
            {
                for (int i = 5; i < 6 + 20; ++i)
                {
                    int base = 13 * 32 + i;
                    matrix_array[base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base]), 0, 255, 0);
                    matrix_array[base + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                }
            }
            else if (selector_y == 1)
            {
                // for (int i = 3; i < 25 + 3; ++i)
                // {
                //     int base = 23 * 32 + i;
                //     matrix_array[base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base]), 0, 255, 0);
                //     matrix_array[base + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32]), 0, 255, 0);
                //     matrix_array[base + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32]), 0, 255, 0);
                //     matrix_array[base + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32]), 0, 255, 0);
                //     matrix_array[base + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32]), 0, 255, 0);
                //     matrix_array[base + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                //     matrix_array[base + 32 + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                // }

                for (int i = 1; i < 31; ++i)
                {
                    int base = 33 * 32 + i;
                    matrix_array[base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base]), 0, 255, 0);
                    matrix_array[base + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                    matrix_array[base + 32 + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
                }
            }
            // else
            // {
            //     for (int i = 1; i < 31; ++i)
            //     {
            //         int base = 33 * 32 + i;
            //         matrix_array[base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base]), 0, 255, 0);
            //         matrix_array[base + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32]), 0, 255, 0);
            //         matrix_array[base + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32]), 0, 255, 0);
            //         matrix_array[base + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32]), 0, 255, 0);
            //         matrix_array[base + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32]), 0, 255, 0);
            //         matrix_array[base + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
            //         matrix_array[base + 32 + 32 + 32 + 32 + 32 + 32] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base + 32 + 32 + 32 + 32 + 32 + 32]), 0, 255, 0);
            //     }
            // }

            update_display();

            button_left = (button_left > 0) ? (button_left - 1) : button_left;
            button_up = (button_up > 0) ? (button_up - 1) : button_up;
            button_down = (button_down > 0) ? (button_down - 1) : button_down;
            button_special = (button_special > 0) ? (button_special - 1) : button_special;
            button_right = (button_right > 0) ? (button_right - 1) : button_right;

            if ((mulberry_rand() & 255) == 255 && button_left == 0)
            {
                button_left = 40;

                int current_x_orig = current_x;
                int current_y_orig = current_y;
                int current_rotation_orig = current_rotation;

                current_rotation = (current_rotation + 1) & 3;

                current_x += (piece_rot_x_shift[current_piece][current_rotation] - piece_rot_x_shift[current_piece][current_rotation_orig]);
                current_y += (piece_rot_y_shift[current_piece][current_rotation] - piece_rot_y_shift[current_piece][current_rotation_orig]);

                current_x = (current_x < 0) ? 0 : current_x;
                current_x = (current_x > piece_max_x[current_piece][current_rotation]) ? piece_max_x[current_piece][current_rotation] : current_x;

                current_y = (current_y < 0) ? 0 : current_y;

                uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation]; // push down
                // no push up for you

                if (current_y > piece_max_y[current_piece][current_rotation] || ((((board[current_y] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0)) // cant rotate!
                {
                    current_rotation = current_rotation_orig;
                    current_x = current_x_orig;
                    current_y = current_y_orig;
                }
            }

            if ((mulberry_rand() & 63) == 63 && button_left == 0)
            {
                button_left = 40;

                if (current_x)
                {
                    --current_x;

                    uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

                    if (((((board[current_y] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0)) // cant move!
                    {
                        ++current_x;
                    }
                }
            }

            if ((mulberry_rand() & 63) == 63 && button_left == 0)
            {
                button_left = 40;

                if (current_x < piece_max_x[current_piece][current_rotation])
                {
                    ++current_x;

                    uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

                    if (((((board[current_y] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0)) // cant move!
                    {
                        --current_x;
                    }
                }
            }

            uint32_t temp_state;

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_5);

            if (temp_state != button_down_state)
            {
                button_down_state = temp_state;
                if (button_down == 0 && button_down_state != 0)
                {
                    selector_y++;
                    selector_y = (selector_y > 1) ? 1 : selector_y;
                    //selector_y = (selector_y > 2) ? 2 : selector_y;
                }
                button_down = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_4);

            if (temp_state != button_up_state)
            {
                button_up_state = temp_state;
                if (button_up == 0 && button_up_state != 0)
                {
                    selector_y--;
                    selector_y = (selector_y < 0) ? 0 : selector_y;
                }
                button_up = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOA->IDR & GPIO_IDR_IDR_10);

            if (temp_state != button_special_state)
            {
                button_special_state = temp_state;
                if (button_special == 0 && button_special_state != 0)
                {
                    frame = 0;

                    if (selector_y == 0)
                    {
                        cur_state = GAME_START_ANIM;

                        for (int i = 0; i < NEXT_PIECES; ++i)
                            next_piece[i] = mulberry_rand() % (uint32_t)7;
                        //current_piece = mulberry_rand() % (uint32_t)7;
                        for (int i = 0; i < 24; ++i)
                            board[i] = 0;
                        for (int i = 0; i < 24; ++i)
                            color_board[i] = 0;
                        current_x = piece_max_x[current_piece][0] >> 1;
                        current_y = 0;
                        current_rotation = 0;
                        score = 0;
                        level = 0;
                        rows_cleared = 0;
                        is_soft_dropping = false;
                        is_hard_dropping = false;
                        moved_on_last_iteration = true;
                        anim_frames = 0;
                        selector_x = 0;
                        selector_y = 0;
                        frame = 0;
                        frames_till_fall = (gravity_table[((level > 29) ? 29 : level)] * (GAME_SPEED_FACTOR_NOM)) / (GAME_SPEED_FACTOR_DENOM);
                        frames_till_fall = (frames_till_fall > 0) ? frames_till_fall : 1;
                        continue;
                    }
                    // else if (selector_y == 1)
                    // {

                    // }
                    else
                    {
                        selector_y = 0;
                        selector_x = 0;
                        cur_state = GAME_LEADERBOARD;
                        continue;
                    }
                }
                button_special = DEBOUNCE_FRAMES;
            }

            ++frame;
        }
        else if (cur_state == GAME_START_ANIM)
        {
            render_frame(BOARD_SHIFT_R - 1 + (BOARD_SHIFT_U - 1) * 32, 22, 42, GAME_BORDER_COLOR);

            int temp = score;
            int base = SCORE_SHIFT_R + SCORE_SHIFT_U * 32 + 6 * 4;

            for (int i = 0; i < 7; ++i, temp /= 10)
            {
                render_char(base, charset[temp % 10], SCORE_COLOR);

                base -= 4;
            }

            base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32;

            for (int y = 0; y < 20; ++y, base += (32 + (32 - 20)))
            {
                uint32_t color_row = color_board[y];
                uint16_t board_row = board[y];

                for (int x = 0; x < 10; ++x, base += 2)
                {
                    uint32_t cur_color = (board_row & 1) ? (colors[color_row & 7]) : (CLEAR_COLOR);

                    matrix_array[base + 0 + 0] = cur_color;
                    matrix_array[base + 1 + 0] = cur_color;
                    matrix_array[base + 0 + 32] = cur_color;
                    matrix_array[base + 1 + 32] = cur_color;

                    color_row >>= 3;
                    board_row >>= 1;
                }
            }

            for(int i = 0; i < NEXT_PIECES; ++i)
            {
                render_tetromino(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 + ((next_piece[i] != 0) ? 1 : 0) + ((next_piece[i] == 1) ? 1 : 0) + ((next_piece[i] != 0) ? (2 * 32) : (3 * 32)), piece_mask[next_piece[i]][0], colors[next_piece[i]]);
                render_frame(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 - 33, 10, 10, NEXT_BORDER_COLOR);
            }

            temp = level;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32 + 4, charset[temp % 10], SCORE_COLOR);
            temp /= 10;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32, charset[temp % 10], SCORE_COLOR);

            if (frame < 1 * 128)
            {
                int mask = charset[3];

                for (int i = 0; i < 5; ++i)
                {
                    for (int u = 0; u < 3; ++u)
                    {
                        if (mask & (1 << (i * 3 + u)))
                        {
                            int base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + (20 - 2 * 3) / 2 + ((40 - 2 * 5) / 2) * 32 + u * 2 + i * 2 * 32;

                            matrix_array[base + 0 + 0] = RGB_SET(255, 0, 0);
                            matrix_array[base + 1 + 0] = RGB_SET(255, 0, 0);
                            matrix_array[base + 0 + 32] = RGB_SET(255, 0, 0);
                            matrix_array[base + 1 + 32] = RGB_SET(255, 0, 0);
                        }
                    }
                }
            }
            else if (frame < 2 * 128)
            {
                int mask = charset[2];

                for (int i = 0; i < 5; ++i)
                {
                    for (int u = 0; u < 3; ++u)
                    {
                        if (mask & (1 << (i * 3 + u)))
                        {
                            int base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + (20 - 2 * 3) / 2 + ((40 - 2 * 5) / 2) * 32 + u * 2 + i * 2 * 32;

                            matrix_array[base + 0 + 0] = RGB_SET(0, 255, 0);
                            matrix_array[base + 1 + 0] = RGB_SET(0, 255, 0);
                            matrix_array[base + 0 + 32] = RGB_SET(0, 255, 0);
                            matrix_array[base + 1 + 32] = RGB_SET(0, 255, 0);
                        }
                    }
                }
            }
            else if (frame < 3 * 128)
            {
                int mask = charset[1];

                for (int i = 0; i < 5; ++i)
                {
                    for (int u = 0; u < 3; ++u)
                    {
                        if (mask & (1 << (i * 3 + u)))
                        {
                            int base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + (20 - 2 * 3) / 2 + ((40 - 2 * 5) / 2) * 32 + u * 2 + i * 2 * 32;

                            matrix_array[base + 0 + 0] = RGB_SET(0, 0, 255);
                            matrix_array[base + 1 + 0] = RGB_SET(0, 0, 255);
                            matrix_array[base + 0 + 32] = RGB_SET(0, 0, 255);
                            matrix_array[base + 1 + 32] = RGB_SET(0, 0, 255);
                        }
                    }
                }
            }
            else
            {
                current_piece = next_piece[0];
                current_x = piece_max_x[current_piece][0] / 2;
                current_y = 0;
                current_rotation = 0;

                for (int i = 0; i < NEXT_PIECES - 1; ++i)
                    next_piece[i] = next_piece[i + 1];

                next_piece[NEXT_PIECES - 1] = mulberry_rand() % (uint32_t)7;
                
                frame = 0;

                cur_state = GAME_INGAME;
            }

            for (volatile int i = 0; i < 1024; ++i)
            ; // we are too fast!

            update_display();

            ++frame;
        }
        else if (cur_state == GAME_INGAME || cur_state == GAME_INGAME_ROWCLEAR_ANIM)
        {
            --frames_till_fall;

            if (frames_till_fall == 0 || is_hard_dropping)
            {
                if (is_soft_dropping)
                {
                    frames_till_fall = SOFT_DROP_SPEED;
                }
                else
                {
                    frames_till_fall = (gravity_table[((level > 29) ? 29 : level)] * (GAME_SPEED_FACTOR_NOM)) / (GAME_SPEED_FACTOR_DENOM);
                    frames_till_fall = (frames_till_fall > 0) ? frames_till_fall : 1;
                }

                uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

                if (((((board[current_y + 1] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 4] >> current_x) & ((cur_piece_mask >> 12)))) != 0) || current_y == piece_max_y[current_piece][current_rotation])
                {
                    if (is_soft_dropping && moved_on_last_iteration) // one more cycle when soft dropping
                    {
                        moved_on_last_iteration = false;
                        frames_till_fall = 100; // or anything else
                        goto __next;
                    }

                    is_hard_dropping = false;

                    board[current_y + 0] |= (((cur_piece_mask) & 0xF) << current_x);
                    board[current_y + 1] |= (((cur_piece_mask >> 4) & 0xF) << current_x);
                    board[current_y + 2] |= (((cur_piece_mask >> 8) & 0xF) << current_x);
                    board[current_y + 3] |= ((cur_piece_mask >> 12) << current_x);

                    if (cur_piece_mask & 1)
                        color_board[current_y + 0] |= (current_piece << (current_x * 3 + 0));
                    if (cur_piece_mask & 2)
                        color_board[current_y + 0] |= (current_piece << (current_x * 3 + 3));
                    if (cur_piece_mask & 4)
                        color_board[current_y + 0] |= (current_piece << (current_x * 3 + 6));
                    if (cur_piece_mask & 8)
                        color_board[current_y + 0] |= (current_piece << (current_x * 3 + 9));

                    if (cur_piece_mask & 16)
                        color_board[current_y + 1] |= (current_piece << (current_x * 3 + 0));
                    if (cur_piece_mask & 32)
                        color_board[current_y + 1] |= (current_piece << (current_x * 3 + 3));
                    if (cur_piece_mask & 64)
                        color_board[current_y + 1] |= (current_piece << (current_x * 3 + 6));

                    if (cur_piece_mask & 256)
                        color_board[current_y + 2] |= (current_piece << (current_x * 3 + 0));
                    if (cur_piece_mask & 512)
                        color_board[current_y + 2] |= (current_piece << (current_x * 3 + 3));

                    if (cur_piece_mask & 4096)
                        color_board[current_y + 3] |= (current_piece << (current_x * 3 + 0));
                    
                    int rc = 0;    

                    for (int k = 0; k < 20; ++k)
                    {
                        if (board[k] == ((1 << 10) - 1))
                            ++rc;
                    }

                    switch (rc)
                    {
                    case 1:
                        score += 40 * (level + 1);
                        rows_cleared += 1;
                        anim_frames += 8;
                        cur_state = GAME_INGAME_ROWCLEAR_ANIM;
                        break;
                    case 2:
                        score += 100 * (level + 1);
                        rows_cleared += 2;
                        anim_frames += 16;
                        cur_state = GAME_INGAME_ROWCLEAR_ANIM;
                        break;
                    case 3:
                        score += 300 * (level + 1);
                        rows_cleared += 3;
                        anim_frames += 32;
                        cur_state = GAME_INGAME_ROWCLEAR_ANIM;
                        break;
                    case 4:
                        score += 1200 * (level + 1);
                        rows_cleared += 4;
                        anim_frames += 64;
                        cur_state = GAME_INGAME_ROWCLEAR_ANIM;
                        break;
                    default:
                        break;
                    }
                    
                    if (rows_cleared >= LEVEL_UP_ROWS)
                    {
                        rows_cleared -= LEVEL_UP_ROWS;
                        ++level;
                    }

                    current_piece = next_piece[0];
                    current_x = piece_max_x[current_piece][0] / 2;
                    current_y = 0;
                    current_rotation = 0;

                    for (int i = 0; i < NEXT_PIECES - 1; ++i)
                        next_piece[i] = next_piece[i + 1];

                    next_piece[NEXT_PIECES - 1] = mulberry_rand() % (uint32_t)7;

                    cur_piece_mask = piece_mask[current_piece][0];

                    if (((((board[current_y] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0)) // cant fit new tetromino
                    {
                        cur_state = GAME_LOST_ANIM;

                        uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

                        board_anim[0] = board[0] & (((cur_piece_mask >> 0) & 0xF) << current_x);
                        board_anim[1] = board[1] & (((cur_piece_mask >> 4) & 0xF) << current_x);
                        board_anim[2] = board[2] & (((cur_piece_mask >> 8) & 0xF) << current_x);
                        board_anim[3] = board[3] & (((cur_piece_mask >> 12) & 0xF) << current_x);

                        for (int i = 4; i < 24; ++i)
                            board_anim[i] = 0;
                        for (int i = 0; i < 24; ++i)
                            color_board_anim[i] = 0;

                        anim_frames = 0;

                        continue;
                    }
                }
                else 
                {
                    ++current_y;
                    moved_on_last_iteration = true;

                    if (is_soft_dropping)
                    {
                        ++score;
                    }
                    else if (is_hard_dropping)
                    {
                        score += 2;
                    }
                }
            }

        __next:

            render_frame(BOARD_SHIFT_R - 1 + (BOARD_SHIFT_U - 1) * 32, 22, 42, GAME_BORDER_COLOR);

            int temp = score;
            int base = SCORE_SHIFT_R + SCORE_SHIFT_U * 32 + 6 * 4;

            for (int i = 0; i < 7; ++i, temp /= 10)
            {
                render_char(base, charset[temp % 10], SCORE_COLOR);

                base -= 4;
            }

            base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32;

            for (int y = 0; y < 20; ++y, base += (32 + (32 - 20)))
            {
                uint32_t color_row = color_board[y];
                uint16_t board_row = board[y];

                for (int x = 0; x < 10; ++x, base += 2)
                {
                    uint32_t cur_color = (board_row & 1) ? (colors[color_row & 7]) : (CLEAR_COLOR);

                    matrix_array[base + 0 + 0] = cur_color;
                    matrix_array[base + 1 + 0] = cur_color;
                    matrix_array[base + 0 + 32] = cur_color;
                    matrix_array[base + 1 + 32] = cur_color;

                    color_row >>= 3;
                    board_row >>= 1;
                }
            }

            uint32_t color = colors[current_piece];
            uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

            for (int current_y_copy = current_y;; ++current_y_copy)
            {
                if (((((board[current_y_copy] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y_copy + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y_copy + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y_copy + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0) || current_y_copy - 1 == piece_max_y[current_piece][current_rotation]) // cant fit new tetromino
                {
                    break;
                }

                render_tetromino(BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + current_y_copy * 32 * 2 + current_x * 2, cur_piece_mask, PIECE_FALL_HIGHLIGHT_COLOR);
            }

            render_tetromino(BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + current_y * 32 * 2 + current_x * 2, cur_piece_mask, color);

            for(int i = 0; i < NEXT_PIECES; ++i)
            {
                render_tetromino(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 + ((next_piece[i] != 0) ? 1 : 0) + ((next_piece[i] == 1) ? 1 : 0) + ((next_piece[i] != 0) ? (2 * 32) : (3 * 32)), piece_mask[next_piece[i]][0], colors[next_piece[i]]);
                render_frame(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 - 33, 10, 10, NEXT_BORDER_COLOR);
            }

            temp = level;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32 + 4, charset[temp % 10], SCORE_COLOR);
            temp /= 10;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32, charset[temp % 10], SCORE_COLOR);

            //render_char(0, charset[(is_soft_dropping) ? 1 : 0], RGB_SET(255, 255, 255));

            if (cur_state == GAME_INGAME_ROWCLEAR_ANIM)
            {
                if (anim_frames & 1)
                {
                    int base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32;

                    for (int y = 0; y < 20; ++y, base += (32 + (32 - 20)))
                    {
                        if (board[y] == ((1 << 10) - 1))
                        {
                            for (int x = 0; x < 10; ++x, base += 2)
                            {
                                matrix_array[base + 0 + 0] = RGB_SET(255, 255, 255);
                                matrix_array[base + 1 + 0] = RGB_SET(255, 255, 255);
                                matrix_array[base + 0 + 32] = RGB_SET(255, 255, 255);
                                matrix_array[base + 1 + 32] = RGB_SET(255, 255, 255);
                            }
                        }
                        else
                            base += 20;
                    }
                }

                --anim_frames;

                if (anim_frames == 0)
                {
                    cur_state = GAME_INGAME;

                    int l = 20 - 1;
                    int k = 20 - 1;

                    for (; k >= 0; k--)
                    {
                        if (board[k] != ((1 << 10) - 1))
                        {
                            board[l] = board[k];
                            color_board[l] = color_board[k];
                            --l;
                        }
                    }

                    for (; l >= 0; l--)
                    {
                        board[l] = 0;
                        color_board[k] = 0;
                    }
                }
            }

            for (volatile int i = 0; i < 1024; ++i)
            ; // we are too fast!

            update_display();

            button_up = (button_up > 0) ? (button_up - 1) : button_up;
            button_down = (button_down > 0) ? (button_down - 1) : button_down;
            button_left = (button_left > 0) ? (button_left - 1) : button_left;
            button_special = (button_special > 0) ? (button_special - 1) : button_special;
            button_right = (button_right > 0) ? (button_right - 1) : button_right;

            uint32_t temp_state;

            // Handle events

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_4);

            if (temp_state != button_down_state) // up
            {
                button_down_state = temp_state;
                is_soft_dropping = button_down_state ? true : false;
                frames_till_fall = (frames_till_fall > SOFT_DROP_SPEED && is_soft_dropping) ? SOFT_DROP_SPEED : frames_till_fall;
            }

            temp_state = (GPIOA->IDR & GPIO_IDR_IDR_10);

            if (temp_state != button_up_state)
            {
                button_up_state = temp_state;
                if (button_up == 0 && button_up_state != 0)
                {
                    int current_x_orig = current_x;
                    int current_y_orig = current_y;
                    int current_rotation_orig = current_rotation;

                    current_rotation = (current_rotation + 1) & 3;

                    current_x += (piece_rot_x_shift[current_piece][current_rotation] - piece_rot_x_shift[current_piece][current_rotation_orig]);
                    current_y += (piece_rot_y_shift[current_piece][current_rotation] - piece_rot_y_shift[current_piece][current_rotation_orig]);

                    current_x = (current_x < 0) ? 0 : current_x;
                    current_x = (current_x > piece_max_x[current_piece][current_rotation]) ? piece_max_x[current_piece][current_rotation] : current_x;

                    current_y = (current_y < 0) ? 0 : current_y;

                    uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation]; // push down
                    // no push up for you

                    if (current_y > piece_max_y[current_piece][current_rotation] || ((((board[current_y] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0)) // cant rotate!
                    {
                        current_rotation = current_rotation_orig;
                        current_x = current_x_orig;
                        current_y = current_y_orig;
                    }
                }
                button_up = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_6);

            if (temp_state != button_left_state)
            {
                button_left_state = temp_state;
                if (button_left == 0 && button_left_state != 0)
                {
                    if (current_x)
                    {
                        --current_x;

                        uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

                        if (((((board[current_y] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0)) // cant move!
                        {
                            ++current_x;
                        }
                    }
                }
                button_left = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_5);

            if (temp_state != button_special_state)
            {
                button_special_state = temp_state;
                if (button_special == 0 && button_special_state != 0)
                {
                    is_hard_dropping = true;
                }
                button_special = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_3);

            if (temp_state != button_right_state)
            {
                button_right_state = temp_state;
                if (button_right == 0 && button_right_state != 0)
                {
                    if (current_x < piece_max_x[current_piece][current_rotation])
                    {
                        ++current_x;

                        uint16_t cur_piece_mask = piece_mask[current_piece][current_rotation];

                        if (((((board[current_y] >> current_x) & (cur_piece_mask & 0xF)) | ((board[current_y + 1] >> current_x) & ((cur_piece_mask >> 4) & 0xF)) | ((board[current_y + 2] >> current_x) & ((cur_piece_mask >> 8) & 0xF)) | ((board[current_y + 3] >> current_x) & ((cur_piece_mask >> 12)))) != 0)) // cant move!
                        {
                            --current_x;
                        }
                    }
                }
                button_right = DEBOUNCE_FRAMES;
            }
        }
        else if (cur_state == GAME_LOST_ANIM)
        {
            uint32_t adv = 0;

            for (int i = 0; i < 20; ++i)
            {
                for (int u = 0; u < 10; ++u)
                {
                    if ((board_anim[i] & (1 << u)) && (color_board_anim[i] & (7 << (3 * u))) != (7 << (3 * u)))
                    {
                        color_board_anim[i] += (1 << (3 * u));

                        adv = 1;
                    }
                    else
                    {
                        uint32_t res = 0;

                        if (i != 0)
                        {
                            res |= (color_board_anim[i - 1] & (7 << (3 * u)));
                        }

                        if (i != 19)
                        {
                            res |= (color_board_anim[i + 1] & (7 << (3 * u)));
                        }

                        if (u != 0)
                        {
                            res |= (color_board_anim[i] & (7 << (3 * (u - 1))));
                        }

                        if (u != 9)
                        {
                            res |= (color_board_anim[i] & (7 << (3 * (u + 1))));
                        }

                        res = !!res;

                        board_anim[i] |= (res << u);
                    }
                }
            }

            anim_frames += !adv;

            if (anim_frames == 100)
            {
                cur_state = GAME_LOST_ANIM_CLEAR;

                anim_frames = 0;
                current_x = 0;
                current_y = 0;
                temp_save.name_length = 0;
                temp_save.score = score;

                frame = 0;

                continue;
            }

            render_frame(BOARD_SHIFT_R - 1 + (BOARD_SHIFT_U - 1) * 32, 22, 42, GAME_BORDER_COLOR);

            int temp = score;
            int base = SCORE_SHIFT_R + SCORE_SHIFT_U * 32 + 6 * 4;

            for (int i = 0; i < 7; ++i, temp /= 10)
            {
                render_char(base, charset[temp % 10], SCORE_COLOR);

                base -= 4;
            }

            render_tetromino(BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + current_y * 32 * 2 + current_x * 2, piece_mask[current_piece][current_rotation], colors[current_piece]);

            base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32;

            for (int y = 0; y < 20; ++y, base += (32 + (32 - 20)))
            {
                uint32_t color_row = color_board[y];
                uint32_t color_row_anim = color_board_anim[y];
                uint16_t board_row = board[y];
                uint16_t board_row_anim = board_anim[y];

                for (int x = 0; x < 10; ++x, base += 2)
                {
                    uint32_t cur_color = (board_row & 1) ? (colors[color_row & 7]) : (CLEAR_COLOR);
                    cur_color = (board_row_anim & 1) ? (RGB_SET((255 * (color_row_anim & 7) / 7), 0, 0)) : cur_color;

                    matrix_array[base + 0 + 0] = cur_color;
                    matrix_array[base + 1 + 0] = cur_color;
                    matrix_array[base + 0 + 32] = cur_color;
                    matrix_array[base + 1 + 32] = cur_color;

                    color_row_anim >>= 3;
                    color_row >>= 3;
                    board_row >>= 1;
                    board_row_anim >>= 1;
                }
            }

            for (int i = 0; i < NEXT_PIECES; ++i)
            {
                render_tetromino(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 + ((next_piece[i] != 0) ? 1 : 0) + ((next_piece[i] == 1) ? 1 : 0) + ((next_piece[i] != 0) ? (2 * 32) : (3 * 32)), piece_mask[next_piece[i]][0], colors[next_piece[i]]);
                render_frame(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 - 33, 10, 10, NEXT_BORDER_COLOR);
            }

            temp = level;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32 + 4, charset[temp % 10], SCORE_COLOR);
            temp /= 10;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32, charset[temp % 10], SCORE_COLOR);

            for (volatile int i = 0; i < (1 << 18); ++i)
                ; // we are too fast!

            update_display();
        }
        else if (cur_state == GAME_LOST_ANIM_CLEAR)
        {
            int cur_anim = (255 - ((anim_frames > 255) ? 255 : anim_frames));

            render_frame(BOARD_SHIFT_R - 1 + (BOARD_SHIFT_U - 1) * 32, 22, 42, RGB_MUL_COMP(GAME_BORDER_COLOR, cur_anim));

            render_tetromino(BOARD_SHIFT_R + BOARD_SHIFT_U * 32 + current_y * 32 * 2 + current_x * 2, piece_mask[current_piece][current_rotation], RGB_MUL_COMP(colors[current_piece], cur_anim));

            int base = BOARD_SHIFT_R + BOARD_SHIFT_U * 32;

            for (int y = 0; y < 20; ++y, base += (32 + (32 - 20))) 
            {
                uint32_t color_row = color_board[y];
                uint32_t color_row_anim = color_board_anim[y];
                uint16_t board_row = board[y];
                uint16_t board_row_anim = board_anim[y];

                for (int x = 0; x < 10; ++x, base += 2)
                {
                    uint32_t cur_color = RGB_MUL_COMP(RGB_SET(255, 0, 0), cur_anim);

                    matrix_array[base + 0 + 0] = cur_color;
                    matrix_array[base + 1 + 0] = cur_color;
                    matrix_array[base + 0 + 32] = cur_color;
                    matrix_array[base + 1 + 32] = cur_color;

                    color_row_anim >>= 3;
                    color_row >>= 3;
                    board_row >>= 1;
                    board_row_anim >>= 1;
                }
            }

            for (int i = 0; i < NEXT_PIECES; ++i)
            {
                render_tetromino(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 + ((next_piece[i] != 0) ? 1 : 0) + ((next_piece[i] == 1) ? 1 : 0) + ((next_piece[i] != 0) ? (2 * 32) : (3 * 32)), piece_mask[next_piece[i]][0], RGB_MUL_COMP(colors[next_piece[i]], cur_anim)); 
                render_frame(NEXT_SHIFT_R + NEXT_SHIFT_U * 32 + i * 9 * 32 - 33, 10, 10, RGB_MUL_COMP(NEXT_BORDER_COLOR, cur_anim));
            }

            int temp = level; 
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32 + 4, charset[temp % 10], RGB_MUL_COMP(SCORE_COLOR, cur_anim));
            temp /= 10;
            render_char(LEVEL_SHIFT_R + LEVEL_SHIFT_U * 32, charset[temp % 10], RGB_MUL_COMP(SCORE_COLOR, cur_anim));

            temp = score;
            base = SCORE_SHIFT_R + SCORE_SHIFT_U * 32 + 6 * 4;

            for (int i = 0; i < 7; ++i, temp /= 10)
            {
                render_char(base, charset[temp % 10], RGB_MUL_COMP(SCORE_COLOR, cur_anim));

                base -= 4;
            }

            render_frame(6 * 32 + 1, 30, 7, RGB_SET(255, 255, 255));

            char *rows[4] = {"   1234567890\x27       ", "   qwertyuiop       ", "   asdfghjkl\x28        ", "   zxcvbnm\x24\x25\x26          "};

            base = (temp_save.name_length < 7) ? (temp_save.name_length * 4) : 24;
            base += (7 * 1) * 32;

            for (int i = 0; i < 5; ++i)
            {
                for (int u = 0; u < 3; ++u)
                {
                    matrix_array[base + i * 32 + u + 2] = ((anim_frames & 128) == 0) ? RGB_SET(32, 255, 32) : 0;
                }
            }

            if (frame)
            {
                int base = (7 * 1) * 32 + 2;
                for (int i = 0; i < 5; ++i)
                {
                    for (int u = 0; u < 28; ++u)
                    {
                        matrix_array[base + i * 32 + u] = RGB_SET(frame * 255 / 255, 0, 0);
                    }
                }
                --frame;
            }

            render_sliding_text((7 * 1) * 32 + 2, temp_save.name, (temp_save.name_length < 7) ? (temp_save.name_length * 4) : 24, (temp_save.name_length < 7) ? 0 : ((temp_save.name_length - 6) * 4), RGB_SET(255, 0, 0));

            render_sliding_text(0, "  highscore speichern", 32, anim_frames >> 5, RGB_MUL_COMP(RGB_SET(255, 255, 255), 255 - cur_anim));
            render_sliding_text((7 * 2) * 32, rows[0], 32, current_x * ((10 * 4)) / 10, RGB_MUL_COMP(RGB_SET(255, 255, 255), 255 - cur_anim));
            render_sliding_text((7 * 3) * 32, rows[1], 32, current_x * ((10 * 4)) / 10, RGB_MUL_COMP(RGB_SET(255, 255, 255), 255 - cur_anim));
            render_sliding_text((7 * 4) * 32, rows[2], 32, current_x * ((10 * 4)) / 10, RGB_MUL_COMP(RGB_SET(255, 255, 255), 255 - cur_anim));
            render_sliding_text((7 * 5) * 32, rows[3], 32, current_x * ((10 * 4)) / 10, RGB_MUL_COMP(RGB_SET(255, 255, 255), 255 - cur_anim));
            render_string((7 * 6) * 32, " fertig", RGB_MUL_COMP(RGB_SET(255, 255, 255), 255 - cur_anim));

            
                
            if (current_y < 4)
            {
                for (int i = 0; i < 5; ++i)
                {
                    for (int u = 0; u < 7; ++u)
                    {
                        int base = (7 * 2 - 1 + current_y * 7 + u) * 32 + 3 * 4 + i - 1;
                        matrix_array[base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base]), 0, 255, 0);
                    }
                }
            }
            else
            {
                for (int i = 0; i < 25; ++i)
                {
                    for (int u = 0; u < 7; ++u)
                    {
                        int base = (7 * 2 - 1 + current_y * 7 + u) * 32 + 1 * 4 + i - 1;
                        matrix_array[base] = RGB_COLOR_MOD(RGB_INVERT(matrix_array[base]), 0, 255, 0);
                    }
                }
            }
            

            for (volatile int i = 0; i < 1024; ++i)
                ; // we are too fast!

            update_display();

            uint32_t temp_state;

            button_up = (button_up > 0) ? (button_up - 1) : button_up;
            button_down = (button_down > 0) ? (button_down - 1) : button_down;
            button_left = (button_left > 0) ? (button_left - 1) : button_left;
            button_right = (button_right > 0) ? (button_right - 1) : button_right;
            button_special = (button_special > 0) ? (button_special - 1) : button_special;

            // Handle events

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_5);

            if (temp_state != button_down_state)
            {
                button_down_state = temp_state;
                if (button_down == 0 && button_down_state != 0)
                {
                    current_y++;
                    current_y = (current_y > 4) ? 4 : current_y;
                    current_x = (current_y != 0 && current_x > 9) ? 9 : current_x;
                }
                button_down = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_4);

            if (temp_state != button_up_state)
            {
                button_up_state = temp_state;
                if (button_up == 0 && button_up_state != 0)
                {
                    current_y--;
                    current_y = (current_y < 0) ? 0 : current_y;
                    current_x = (current_y != 0 && current_x > 9) ? 9 : current_x;
                }
                button_up = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_6);

            if (temp_state != button_left_state)
            {
                button_left_state = temp_state;
                if (button_left == 0 && button_left_state != 0)
                {
                    if (current_y != 4)
                        current_x--;
                    current_x = (current_x < 0) ? 0 : current_x;
                }
                button_left = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOB->IDR & GPIO_IDR_IDR_3);

            if (temp_state != button_right_state)
            {
                button_right_state = temp_state;
                if (button_right == 0 && button_right_state != 0)
                {
                    button_right = DEBOUNCE_FRAMES;
                    if (current_y != 4)
                        current_x++;
                    current_x = (current_x > 10) ? 10 : current_x;
                    current_x = (current_y != 0 && current_x > 9) ? 9 : current_x;
                }
                button_right = DEBOUNCE_FRAMES;
            }

            temp_state = (GPIOA->IDR & GPIO_IDR_IDR_10);

            if (temp_state != button_special_state)
            {
                button_special_state = temp_state;
                if (button_special == 0 && button_special_state != 0)
                {
                    if (current_y == 4)
                    {
                        if (temp_save.name_length != 0)
                        {
                            cur_state = GAME_MENU;

                            selector_y = 0;
                            frame = 0;

                            int i = 0;

                            while (i < save_count && temp_save.score <= save_buffer[i].score)
                            {
                                ++i;
                            }

                            for (int j = save_count; j > i; --j)
                            {
                                save_buffer[j] = save_buffer[j - 1];
                            }

                            save_buffer[i] = temp_save;

                            if (save_count < MAX_SAVES)
                            {
                                ++save_count;
                            }

                            continue;
                        }
                        else
                        {
                            frame = 128;
                        }
                    }
                    else
                    {
                        if (current_y == 0 && current_x == 10)
                        {
                            --temp_save.name_length;
                            temp_save.name_length = (temp_save.name_length < 0) ? 0 : temp_save.name_length;
                        }
                        else if (temp_save.name_length < SAVE_NAME_MAX_LENGTH)
                        {
                            temp_save.name[temp_save.name_length++] = rows[current_y][current_x + 3];
                        }
                    }
                }
                button_special = DEBOUNCE_FRAMES;
            }

            ++anim_frames;
        }
    }
}
