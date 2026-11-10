/* A public-header-only consumer for the installed package. */
#include <nesturbator.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    VIDEO_WIDTH = 256,
    VIDEO_HEIGHT = 240,
    AUDIO_CAPACITY = 799,
    GAMEPLAY_FRAMES = 110,
    AUDIO_FIRST_FRAME = 61,
    AUDIO_LAST_FRAME = 76,
    A_PULSE_LAST_FRAME = 57,
    A_PULSE_INTERVAL = 3,
    RIGHT_FIRST_FRAME = 80,
    RIGHT_LAST_FRAME = 110,
    PLAYER_BEFORE_FRAME = 80,
    PLAYER_AFTER_FRAME = 95,
    REFUSAL_CONTINUATION_FRAMES = GAMEPLAY_FRAMES - PLAYER_BEFORE_FRAME,
    VERDICT_GAME_STATE = 1,
    VERDICT_INPUT_PROGRESS = 2,
    VERDICT_VISIBLE_PIXELS = 4,
    VERDICT_GAMEPLAY_PCM = 8
};

static uint16_t video[VIDEO_WIDTH * VIDEO_HEIGHT];
static uint16_t twin_video[VIDEO_WIDTH * VIDEO_HEIGHT];
static uint16_t before_video[VIDEO_WIDTH * VIDEO_HEIGHT];
static uint16_t after_video[VIDEO_WIDTH * VIDEO_HEIGHT];
static int16_t audio[AUDIO_CAPACITY];
static int16_t twin_audio[AUDIO_CAPACITY];

typedef struct gameplay_observation {
    uint8_t attract_mode;
    uint8_t player_enabled;
    uint8_t player_x;
    uint8_t player_y;
    uint8_t level_number;
} gameplay_observation;

typedef struct gameplay_result {
    const char *scenario;
    uint32_t completed_frames;
    uint32_t gameplay_entry_frame;
    gameplay_observation final_state;
    gameplay_observation before_motion;
    gameplay_observation after_motion;
    uint32_t audio_sample_count;
    uint32_t audio_nonzero_samples;
    int32_t audio_peak_abs;
    uint64_t audio_hash;
    uint64_t before_pixel_hash;
    uint64_t after_pixel_hash;
    uint32_t changed_pixels_in_player_sweep;
    uint32_t roi_x;
    uint32_t roi_y;
    uint32_t roi_width;
    uint32_t roi_height;
    uint32_t observed_pixel_x;
    uint32_t observed_pixel_y;
    uint32_t audio_count_min;
    uint32_t audio_count_max;
    uint32_t failure_mask;
    nesturbator_status refused_load_status;
    uint32_t continuation_frames;
    uint8_t continued_player_x;
    int game_state_ok;
    int input_progress_ok;
    int visible_pixels_ok;
    int gameplay_pcm_ok;
    int refusal_preserved;
    int continuation_equal;
    int buffer_edges_ok;
    int synthetic_controls_ok;
    int synthetic_game_state_control_ok;
    int synthetic_input_control_ok;
    int synthetic_pixel_control_ok;
    int synthetic_pcm_control_ok;
} gameplay_result;

enum gameplay_scenario { SCENARIO_POSITIVE, SCENARIO_NO_INPUT, SCENARIO_START_ONLY };

static uint64_t hash_pixels(const uint16_t *pixels)
{
    uint64_t hash = UINT64_C(14695981039346656037);
    size_t i;
    for (i = 0; i < (size_t)VIDEO_WIDTH * VIDEO_HEIGHT; ++i) {
        hash ^= pixels[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int read_rom(const char *path, uint8_t **bytes, size_t *size)
{
    FILE *file = fopen(path, "rb");
    long length;
    uint8_t *data;
    size_t bytes_read;
    int close_result;
    if (file == NULL)
        return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (data == NULL) {
        fclose(file);
        return 0;
    }
    bytes_read = fread(data, 1, (size_t)length, file);
    close_result = fclose(file);
    if (bytes_read != (size_t)length || close_result != 0) {
        free(data);
        return 0;
    }
    *bytes = data;
    *size = (size_t)length;
    return 1;
}

static int read_ram(const nesturbator *inst, uint16_t address, uint8_t *out)
{
    return nesturbator_peek_cpu_ram(inst, address, out) == NESTURBATOR_OK;
}

static int observe_game(const nesturbator *inst, gameplay_observation *out)
{
    return read_ram(inst, 0x0011u, &out->attract_mode) &&
           read_ram(inst, 0x0044u, &out->player_enabled) &&
           read_ram(inst, 0x0048u, &out->player_x) && read_ram(inst, 0x004cu, &out->player_y) &&
           read_ram(inst, 0x00ddu, &out->level_number);
}

static int create_loaded_instance(const uint8_t *rom, size_t rom_size, nesturbator **out)
{
    nesturbator_config config;
    nesturbator *inst = NULL;
    nesturbator_status status;
    memset(&config, 0, sizeof config);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    status = nesturbator_create(&config, &inst);
    if (status != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator_create: status %d\n", (int)status);
        return 0;
    }
    status = nesturbator_load_cartridge(inst, rom, rom_size);
    if (status != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator_load_cartridge: status %d\n", (int)status);
        nesturbator_destroy(inst);
        return 0;
    }
    *out = inst;
    return 1;
}

static uint8_t buttons_for_frame(uint32_t frame, enum gameplay_scenario scenario)
{
    if (scenario == SCENARIO_NO_INPUT)
        return 0u;
    if (scenario == SCENARIO_START_ONLY) {
        if (frame <= A_PULSE_LAST_FRAME && frame % A_PULSE_INTERVAL == 0u)
            return NESTURBATOR_BUTTON_START;
        return 0u;
    }
    if (frame <= A_PULSE_LAST_FRAME && frame % A_PULSE_INTERVAL == 0u)
        return NESTURBATOR_BUTTON_A;
    if (frame >= RIGHT_FIRST_FRAME && frame <= RIGHT_LAST_FRAME)
        return NESTURBATOR_BUTTON_RIGHT;
    return 0u;
}

static nesturbator_status run_frame_with_input(nesturbator *inst, uint8_t port0, uint8_t port1,
                                               uint16_t *pixels, uint32_t pitch, int16_t *samples,
                                               uint32_t capacity, nesturbator_frame *out)
{
    nesturbator_input input;
    nesturbator_status status;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = port0;
    input.buttons[1] = port1;
    status = nesturbator_set_input(inst, &input);
    if (status != NESTURBATOR_OK)
        return status;
    memset(out, 0, sizeof *out);
    out->size = (uint32_t)sizeof *out;
    out->video = pixels;
    out->video_pitch = pitch;
    out->audio = samples;
    out->audio_capacity = capacity;
    return nesturbator_run_frame(inst, out);
}

static int same_info(const nesturbator *left, const nesturbator *right)
{
    nesturbator_info left_info;
    nesturbator_info right_info;
    memset(&left_info, 0, sizeof left_info);
    memset(&right_info, 0, sizeof right_info);
    left_info.size = (uint32_t)sizeof left_info;
    right_info.size = (uint32_t)sizeof right_info;
    nesturbator_get_info(left, &left_info);
    nesturbator_get_info(right, &right_info);
    return left_info.width == right_info.width && left_info.height == right_info.height &&
           left_info.fps_num == right_info.fps_num && left_info.fps_den == right_info.fps_den &&
           left_info.sample_rate_num == right_info.sample_rate_num &&
           left_info.sample_rate_den == right_info.sample_rate_den;
}

static int same_game_canaries(const nesturbator *left, const nesturbator *right)
{
    gameplay_observation left_state;
    gameplay_observation right_state;
    return observe_game(left, &left_state) && observe_game(right, &right_state) &&
           memcmp(&left_state, &right_state, sizeof left_state) == 0;
}

static int same_frame_output(const nesturbator_frame *left_frame, const uint16_t *left_pixels,
                             const int16_t *left_audio, const nesturbator_frame *right_frame,
                             const uint16_t *right_pixels, const int16_t *right_audio)
{
    if (left_frame->audio_count > AUDIO_CAPACITY || right_frame->audio_count > AUDIO_CAPACITY)
        return 0;
    return left_frame->frame_number == right_frame->frame_number &&
           left_frame->ticks == right_frame->ticks &&
           left_frame->audio_count == right_frame->audio_count &&
           memcmp(left_pixels, right_pixels, sizeof video) == 0 &&
           memcmp(left_audio, right_audio,
                  (size_t)left_frame->audio_count * sizeof left_audio[0]) == 0;
}

static uint32_t count_roi_differences(const uint16_t *before, const uint16_t *after, uint32_t x,
                                      uint32_t y, uint32_t width, uint32_t height)
{
    uint32_t changed = 0u;
    uint32_t row;
    uint32_t column;
    for (row = y; row < y + height; ++row) {
        for (column = x; column < x + width; ++column) {
            size_t index = (size_t)row * VIDEO_WIDTH + column;
            if (before[index] != after[index])
                ++changed;
        }
    }
    return changed;
}

static void write_json_bool(FILE *file, int value)
{
    fputs(value ? "true" : "false", file);
}

static uint32_t verdict_failure_mask(const gameplay_result *result)
{
    uint32_t failures = 0u;
    if (!result->game_state_ok)
        failures |= VERDICT_GAME_STATE;
    if (!result->input_progress_ok)
        failures |= VERDICT_INPUT_PROGRESS;
    if (!result->visible_pixels_ok)
        failures |= VERDICT_VISIBLE_PIXELS;
    if (!result->gameplay_pcm_ok)
        failures |= VERDICT_GAMEPLAY_PCM;
    return failures;
}

static void write_failed_predicates(FILE *file, uint32_t failures)
{
    int comma = 0;
    fputc('[', file);
#define WRITE_FAILURE(bit, name)                                                                   \
    do {                                                                                           \
        if ((failures & (bit)) != 0u) {                                                            \
            if (comma)                                                                             \
                fputs(", ", file);                                                                 \
            fprintf(file, "\"%s\"", (name));                                                       \
            comma = 1;                                                                             \
        }                                                                                          \
    } while (0)
    WRITE_FAILURE(VERDICT_GAME_STATE, "game_state");
    WRITE_FAILURE(VERDICT_INPUT_PROGRESS, "input_caused_progress");
    WRITE_FAILURE(VERDICT_VISIBLE_PIXELS, "visible_native_response");
    WRITE_FAILURE(VERDICT_GAMEPLAY_PCM, "non_silent_gameplay_pcm");
#undef WRITE_FAILURE
    fputc(']', file);
}

static int write_gameplay_report(const char *path, const nesturbator_version *version,
                                 const nesturbator_info *info, const gameplay_result *result,
                                 int integration_checks_ok)
{
    FILE *file = fopen(path, "wb");
    int positive = strcmp(result->scenario, "positive") == 0;
    int passed = result->failure_mask == 0u && integration_checks_ok;
    int32_t player_delta =
        (int32_t)result->after_motion.player_x - (int32_t)result->before_motion.player_x;
    if (file == NULL)
        return 0;
    fprintf(file,
            "{\n"
            "  \"game\": \"DABG\",\n"
            "  \"candidate_status\": \"unpublished\",\n"
            "  \"scenario\": \"%s\",\n"
            "  \"header_version\": {\"major\": %u, \"minor\": %u, \"patch\": %u, "
            "\"abi\": %u, \"behaviour_revision\": %u, \"state_format\": %u},\n"
            "  \"library_version\": {\"major\": %u, \"minor\": %u, \"patch\": %u, "
            "\"abi\": %u, \"behaviour_revision\": %u},\n"
            "  \"route\": \"ntsc-only-base-api\",\n"
            "  \"video\": {\"width\": %u, \"height\": %u},\n"
            "  \"rates\": {\"fps_num\": %u, \"fps_den\": %u, "
            "\"sample_rate_num\": %u, \"sample_rate_den\": %u},\n"
            "  \"completed_frames\": %u,\n",
            result->scenario, (unsigned)NESTURBATOR_VERSION_MAJOR,
            (unsigned)NESTURBATOR_VERSION_MINOR, (unsigned)NESTURBATOR_VERSION_PATCH,
            (unsigned)NESTURBATOR_ABI_VERSION, (unsigned)NESTURBATOR_BEHAVIOUR_REVISION,
            (unsigned)NESTURBATOR_STATE_FORMAT_VERSION, version->major, version->minor,
            version->patch, version->abi, version->behaviour_revision, info->width, info->height,
            info->fps_num, info->fps_den, info->sample_rate_num, info->sample_rate_den,
            result->completed_frames);
    if (positive) {
        fprintf(file,
                "  \"input_recipe\": {\"one_based_frames\": true, "
                "\"port0_a_mask\": %u, \"a_pulse_frames\": [3, 57, 3], "
                "\"port0_right_mask\": %u, \"right_frames\": [80, 110], "
                "\"neutral_frames\": [58, 79], \"port1_mask\": 0},\n",
                (unsigned)NESTURBATOR_BUTTON_A, (unsigned)NESTURBATOR_BUTTON_RIGHT);
    } else if (strcmp(result->scenario, "no-input") == 0) {
        fputs("  \"input_recipe\": {\"one_based_frames\": true, "
              "\"port0_mask\": 0, \"frames\": [1, 110], \"port1_mask\": 0},\n",
              file);
    } else {
        fprintf(file,
                "  \"input_recipe\": {\"one_based_frames\": true, "
                "\"port0_start_mask\": %u, \"start_pulse_frames\": [3, 57, 3], "
                "\"neutral_frames\": [58, 110], \"port1_mask\": 0},\n",
                (unsigned)NESTURBATOR_BUTTON_START);
    }
    fputs("  \"ram_allowlist\": {\"attract_mode\": 17, \"player_enabled_port0\": 68, "
          "\"player_x_port0\": 72, \"player_y_port0\": 76, \"level_number\": 221},\n",
          file);
    fprintf(file,
            "  \"state\": {\"attract_mode\": %u, \"player_enabled\": %u, "
            "\"level_number\": %u, \"entry_frame\": %u},\n"
            "  \"input_progress\": {\"port0_mask\": %u, \"port1_mask\": 0, "
            "\"first_frame\": %u, \"last_frame\": %u, "
            "\"player_x_before\": %u, \"player_x_after\": %u, \"player_x_delta\": %d},\n"
            "  \"native_pixels\": {\"player_x\": %u, \"player_y\": %u, "
            "\"before_hash_fnv1a64\": \"%016llx\", \"after_hash_fnv1a64\": \"%016llx\", "
            "\"roi\": [%u, %u, %u, %u], \"changed_pixels\": %u},\n"
            "  \"pcm\": {\"first_frame\": %u, \"last_frame\": %u, "
            "\"samples\": %u, \"nonzero_samples\": %u, \"peak_abs\": %d, "
            "\"audio_count_min\": %u, \"audio_count_max\": %u, "
            "\"hash_fnv1a64\": \"%016llx\"},\n"
            "  \"predicates\": {\"game_state\": ",
            (unsigned)result->final_state.attract_mode,
            (unsigned)result->final_state.player_enabled,
            (unsigned)result->final_state.level_number, result->gameplay_entry_frame,
            (unsigned)buttons_for_frame(PLAYER_BEFORE_FRAME,
                                        strcmp(result->scenario, "positive") == 0
                                            ? SCENARIO_POSITIVE
                                            : (strcmp(result->scenario, "no-input") == 0
                                                   ? SCENARIO_NO_INPUT
                                                   : SCENARIO_START_ONLY)),
            (unsigned)PLAYER_BEFORE_FRAME, (unsigned)PLAYER_AFTER_FRAME,
            (unsigned)result->before_motion.player_x, (unsigned)result->after_motion.player_x,
            player_delta, result->observed_pixel_x, result->observed_pixel_y,
            (unsigned long long)result->before_pixel_hash,
            (unsigned long long)result->after_pixel_hash, result->roi_x, result->roi_y,
            result->roi_width, result->roi_height, result->changed_pixels_in_player_sweep,
            (unsigned)AUDIO_FIRST_FRAME, (unsigned)AUDIO_LAST_FRAME, result->audio_sample_count,
            result->audio_nonzero_samples, result->audio_peak_abs, result->audio_count_min,
            result->audio_count_max, (unsigned long long)result->audio_hash);
    write_json_bool(file, result->game_state_ok);
    fputs(", \"input_caused_progress\": ", file);
    write_json_bool(file, result->input_progress_ok);
    fputs(", \"visible_native_response\": ", file);
    write_json_bool(file, result->visible_pixels_ok);
    fputs(", \"non_silent_gameplay_pcm\": ", file);
    write_json_bool(file, result->gameplay_pcm_ok);
    fprintf(file, "},\n  \"failure_mask\": %u,\n  \"failed_predicates\": ", result->failure_mask);
    write_failed_predicates(file, result->failure_mask);
    fprintf(file,
            ",\n  \"integrity_checks\": {\"executed\": %s, "
            "\"refused_load_status\": %d, \"refusal_preserved\": %s, "
            "\"continuation_equal\": %s, \"continuation_frames\": %u, "
            "\"continued_player_x\": %u, \"buffer_edges_passed\": %s, "
            "\"synthetic_controls\": {\"missing_game_state_rejected\": %s, "
            "\"missing_input_progress_rejected\": %s, \"missing_pixels_rejected\": %s, "
            "\"silent_pcm_rejected\": %s}, \"all_positive_checks_passed\": %s, "
            "\"semantic_control_completed\": %s},\n"
            "  \"passed\": %s\n}\n",
            positive ? "true" : "false", (int)result->refused_load_status,
            result->refusal_preserved ? "true" : "false",
            result->continuation_equal ? "true" : "false", result->continuation_frames,
            (unsigned)result->continued_player_x, result->buffer_edges_ok ? "true" : "false",
            result->synthetic_game_state_control_ok ? "true" : "false",
            result->synthetic_input_control_ok ? "true" : "false",
            result->synthetic_pixel_control_ok ? "true" : "false",
            result->synthetic_pcm_control_ok ? "true" : "false",
            positive && integration_checks_ok ? "true" : "false",
            !positive && integration_checks_ok ? "true" : "false", passed ? "true" : "false");
    return fclose(file) == 0;
}

static int run_smoke(void)
{
    nesturbator_config config;
    nesturbator *inst = NULL;
    nesturbator_frame frame;
    nesturbator_status status;
    memset(&config, 0, sizeof config);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    status = nesturbator_create(&config, &inst);
    if (status != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator_create: status %d\n", (int)status);
        return 1;
    }
    memset(&frame, 0, sizeof frame);
    frame.size = (uint32_t)sizeof frame;
    frame.video = video;
    frame.video_pitch = VIDEO_WIDTH;
    frame.audio = audio;
    frame.audio_capacity = AUDIO_CAPACITY;
    status = nesturbator_run_frame(inst, &frame);
    nesturbator_destroy(inst);
    if (status != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator_run_frame: status %d\n", (int)status);
        return 1;
    }
    if (frame.frame_number != 1u) {
        fprintf(stderr, "frame_number %llu, expected 1\n", (unsigned long long)frame.frame_number);
        return 1;
    }
    return 0;
}

static int refused_call_preserved_outputs(nesturbator *inst, uint8_t port0, uint32_t pitch,
                                          int use_audio, uint32_t capacity,
                                          nesturbator_status expected_status)
{
    nesturbator_input input;
    nesturbator_frame frame;
    nesturbator_status status;
    size_t i;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = port0;
    status = nesturbator_set_input(inst, &input);
    if (status != NESTURBATOR_OK)
        return 0;
    for (i = 0u; i < (size_t)VIDEO_WIDTH * VIDEO_HEIGHT; ++i)
        video[i] = UINT16_C(0x5a5a);
    for (i = 0u; i < AUDIO_CAPACITY; ++i)
        audio[i] = INT16_C(0x2323);
    memset(&frame, 0, sizeof frame);
    frame.size = (uint32_t)sizeof frame;
    frame.video = video;
    frame.video_pitch = pitch;
    frame.audio = use_audio ? audio : NULL;
    frame.audio_capacity = capacity;
    frame.audio_count = UINT32_C(0xa17d);
    frame.frame_number = UINT64_C(0x1122334455667788);
    frame.ticks = UINT64_C(0x8877665544332211);
    status = nesturbator_run_frame(inst, &frame);
    if (status != expected_status || frame.audio_count != UINT32_C(0xa17d) ||
        frame.frame_number != UINT64_C(0x1122334455667788) ||
        frame.ticks != UINT64_C(0x8877665544332211))
        return 0;
    for (i = 0u; i < (size_t)VIDEO_WIDTH * VIDEO_HEIGHT; ++i) {
        if (video[i] != UINT16_C(0x5a5a))
            return 0;
    }
    if (use_audio) {
        for (i = 0u; i < AUDIO_CAPACITY; ++i) {
            if (audio[i] != INT16_C(0x2323))
                return 0;
        }
    }
    return 1;
}

static int run_buffer_edge_controls(const uint8_t *rom, size_t rom_size)
{
    nesturbator *target = NULL;
    nesturbator *reference = NULL;
    uint32_t frame_index;
    unsigned edge;
    int ok = 0;
    if (!create_loaded_instance(rom, rom_size, &target) ||
        !create_loaded_instance(rom, rom_size, &reference))
        goto done;
    for (frame_index = 1u; frame_index <= PLAYER_BEFORE_FRAME; ++frame_index) {
        nesturbator_frame target_frame;
        nesturbator_frame reference_frame;
        uint8_t mask = buttons_for_frame(frame_index, SCENARIO_POSITIVE);
        if (run_frame_with_input(target, mask, 0u, video, VIDEO_WIDTH, audio, AUDIO_CAPACITY,
                                 &target_frame) != NESTURBATOR_OK ||
            target_frame.audio_count > AUDIO_CAPACITY ||
            run_frame_with_input(reference, mask, 0u, twin_video, VIDEO_WIDTH, twin_audio,
                                 AUDIO_CAPACITY, &reference_frame) != NESTURBATOR_OK ||
            reference_frame.audio_count > AUDIO_CAPACITY ||
            !same_frame_output(&target_frame, video, audio, &reference_frame, twin_video,
                               twin_audio) ||
            !same_game_canaries(target, reference))
            goto done;
    }
    for (edge = 0u; edge < 4u; ++edge) {
        nesturbator_frame reference_frame;
        nesturbator_frame target_frame;
        nesturbator_status expected_status;
        uint32_t pitch = VIDEO_WIDTH;
        uint32_t capacity;
        int use_audio = 1;
        uint8_t input_mask = NESTURBATOR_BUTTON_RIGHT;
        if (run_frame_with_input(reference, input_mask, 0u, twin_video, VIDEO_WIDTH, twin_audio,
                                 AUDIO_CAPACITY, &reference_frame) != NESTURBATOR_OK ||
            reference_frame.audio_count > AUDIO_CAPACITY)
            goto done;
        if (edge == 0u) {
            if (reference_frame.audio_count == 0u)
                goto done;
            capacity = reference_frame.audio_count - 1u;
            expected_status = NESTURBATOR_ERR_BUFFER_TOO_SMALL;
        } else if (edge == 1u) {
            pitch = VIDEO_WIDTH - 1u;
            capacity = reference_frame.audio_count;
            expected_status = NESTURBATOR_ERR_BUFFER_TOO_SMALL;
        } else if (edge == 2u) {
            capacity = AUDIO_CAPACITY;
            use_audio = 0;
            expected_status = NESTURBATOR_ERR_ARGUMENT;
        } else {
            capacity = 0u;
            use_audio = 0;
            expected_status = NESTURBATOR_ERR_BUFFER_TOO_SMALL;
        }
        if (!refused_call_preserved_outputs(target, input_mask, pitch, use_audio, capacity,
                                            expected_status))
            goto done;
        if (run_frame_with_input(target, input_mask, 0u, video, VIDEO_WIDTH, audio,
                                 reference_frame.audio_count, &target_frame) != NESTURBATOR_OK ||
            target_frame.audio_count > AUDIO_CAPACITY ||
            !same_frame_output(&target_frame, video, audio, &reference_frame, twin_video,
                               twin_audio) ||
            !same_info(target, reference) || !same_game_canaries(target, reference))
            goto done;
    }
    ok = 1;
done:
    nesturbator_destroy(target);
    nesturbator_destroy(reference);
    return ok;
}

static int refuse_bad_replacement(nesturbator *active, nesturbator *twin, const uint8_t *rom,
                                  size_t rom_size, gameplay_result *result)
{
    uint8_t *malformed = (uint8_t *)malloc(rom_size);
    gameplay_observation active_state;
    nesturbator_status status;
    int before_matches;
    int after_matches;
    if (malformed == NULL || rom_size == 0u) {
        free(malformed);
        return 0;
    }
    memcpy(malformed, rom, rom_size);
    malformed[0] ^= 1u;
    before_matches = observe_game(active, &active_state) && active_state.attract_mode == 0u &&
                     active_state.player_enabled == 1u && active_state.level_number == 1u &&
                     same_info(active, twin) && same_game_canaries(active, twin);
    if (!before_matches) {
        free(malformed);
        return 0;
    }
    status = nesturbator_load_cartridge(active, malformed, rom_size);
    free(malformed);
    result->refused_load_status = status;
    after_matches = status == NESTURBATOR_ERR_CARTRIDGE && same_info(active, twin) &&
                    same_game_canaries(active, twin);
    result->refusal_preserved = after_matches;
    return after_matches;
}

static int run_synthetic_verdict_controls(gameplay_result *positive)
{
    gameplay_result altered;
    altered = *positive;
    altered.game_state_ok = 0;
    positive->synthetic_game_state_control_ok =
        verdict_failure_mask(&altered) == VERDICT_GAME_STATE;
    altered = *positive;
    altered.input_progress_ok = 0;
    positive->synthetic_input_control_ok = verdict_failure_mask(&altered) == VERDICT_INPUT_PROGRESS;
    altered = *positive;
    altered.visible_pixels_ok = 0;
    positive->synthetic_pixel_control_ok = verdict_failure_mask(&altered) == VERDICT_VISIBLE_PIXELS;
    altered = *positive;
    altered.gameplay_pcm_ok = 0;
    positive->synthetic_pcm_control_ok = verdict_failure_mask(&altered) == VERDICT_GAMEPLAY_PCM;
    return positive->synthetic_game_state_control_ok && positive->synthetic_input_control_ok &&
           positive->synthetic_pixel_control_ok && positive->synthetic_pcm_control_ok;
}

static int run_gameplay_scenario(const char *rom_path, const char *report_path,
                                 enum gameplay_scenario scenario)
{
    uint8_t *rom = NULL;
    size_t rom_size = 0u;
    nesturbator *inst = NULL;
    nesturbator *twin = NULL;
    nesturbator_version version;
    nesturbator_info info;
    gameplay_result result;
    uint32_t frame_index;
    int integration_checks_ok = 0;
    int return_code = 1;

    memset(&result, 0, sizeof result);
    result.scenario = scenario == SCENARIO_POSITIVE   ? "positive"
                      : scenario == SCENARIO_NO_INPUT ? "no-input"
                                                      : "start-only";
    result.audio_count_min = UINT32_MAX;
    result.audio_hash = UINT64_C(14695981039346656037);
    if (!read_rom(rom_path, &rom, &rom_size)) {
        fprintf(stderr, "could not read selected ROM\n");
        goto done;
    }
    if (!create_loaded_instance(rom, rom_size, &inst))
        goto done;
    if (scenario == SCENARIO_POSITIVE && !create_loaded_instance(rom, rom_size, &twin))
        goto done;
    memset(&version, 0, sizeof version);
    version.size = (uint32_t)sizeof version;
    nesturbator_get_version(&version);
    memset(&info, 0, sizeof info);
    info.size = (uint32_t)sizeof info;
    nesturbator_get_info(inst, &info);
    if (info.width != VIDEO_WIDTH || info.height != VIDEO_HEIGHT || info.fps_num != 39375000u ||
        info.fps_den != 655171u || info.sample_rate_num != 48000u || info.sample_rate_den != 1u) {
        fprintf(stderr, "gameplay metadata predicate failed: expected NTSC 256x240, 48000/1\n");
        goto done;
    }

    for (frame_index = 1u; frame_index <= GAMEPLAY_FRAMES; ++frame_index) {
        nesturbator_frame frame;
        nesturbator_frame twin_frame;
        nesturbator_status status;
        uint8_t mask = buttons_for_frame(frame_index, scenario);
        uint32_t sample_index;
        status =
            run_frame_with_input(inst, mask, 0u, video, VIDEO_WIDTH, audio, AUDIO_CAPACITY, &frame);
        if (status != NESTURBATOR_OK) {
            fprintf(stderr, "emulator frame %u failed: status %d\n", frame_index, (int)status);
            goto done;
        }
        if (frame.audio_count > AUDIO_CAPACITY) {
            fprintf(stderr, "emulator frame %u returned audio_count %u beyond capacity %u\n",
                    frame_index, frame.audio_count, (unsigned)AUDIO_CAPACITY);
            goto done;
        }
        if (twin != NULL) {
            status = run_frame_with_input(twin, mask, 0u, twin_video, VIDEO_WIDTH, twin_audio,
                                          AUDIO_CAPACITY, &twin_frame);
            if (status != NESTURBATOR_OK || twin_frame.audio_count > AUDIO_CAPACITY ||
                !same_frame_output(&frame, video, audio, &twin_frame, twin_video, twin_audio) ||
                !same_info(inst, twin) || !same_game_canaries(inst, twin)) {
                fprintf(stderr, "pre/post-refusal twin diverged at frame %u\n", frame_index);
                goto done;
            }
        }
        result.completed_frames = frame.frame_number;
        if (frame_index == PLAYER_BEFORE_FRAME) {
            memcpy(before_video, video, sizeof before_video);
            result.before_pixel_hash = hash_pixels(video);
            if (!observe_game(inst, &result.before_motion)) {
                fprintf(stderr, "RAM observation failed at frame %u\n", frame_index);
                goto done;
            }
            if (twin != NULL && !refuse_bad_replacement(inst, twin, rom, rom_size, &result)) {
                fprintf(stderr, "malformed active-game replacement was not refused atomically\n");
                goto done;
            }
        }
        if (frame_index == PLAYER_AFTER_FRAME) {
            memcpy(after_video, video, sizeof after_video);
            result.after_pixel_hash = hash_pixels(after_video);
            if (!observe_game(inst, &result.after_motion)) {
                fprintf(stderr, "RAM observation failed at frame %u\n", frame_index);
                goto done;
            }
        }
        if (frame_index >= AUDIO_FIRST_FRAME && frame_index <= AUDIO_LAST_FRAME) {
            result.audio_sample_count += frame.audio_count;
            if (frame.audio_count < result.audio_count_min)
                result.audio_count_min = frame.audio_count;
            if (frame.audio_count > result.audio_count_max)
                result.audio_count_max = frame.audio_count;
            for (sample_index = 0u; sample_index < frame.audio_count; ++sample_index) {
                int32_t value = audio[sample_index];
                int32_t magnitude = value < 0 ? -value : value;
                result.audio_hash ^= (uint16_t)audio[sample_index];
                result.audio_hash *= UINT64_C(1099511628211);
                if (value != 0)
                    ++result.audio_nonzero_samples;
                if (magnitude > result.audio_peak_abs)
                    result.audio_peak_abs = magnitude;
            }
        }
        if (result.gameplay_entry_frame == 0u) {
            gameplay_observation observation;
            if (!observe_game(inst, &observation)) {
                fprintf(stderr, "RAM observation failed at frame %u\n", frame_index);
                goto done;
            }
            if (observation.attract_mode == 0u && observation.player_enabled == 1u &&
                observation.level_number == 1u)
                result.gameplay_entry_frame = frame_index;
        }
        if (frame_index > PLAYER_BEFORE_FRAME && twin != NULL) {
            ++result.continuation_frames;
            result.continuation_equal = 1;
        }
    }
    if (!observe_game(inst, &result.final_state)) {
        fprintf(stderr, "final RAM observation failed\n");
        goto done;
    }
    result.continued_player_x = result.final_state.player_x;
    result.observed_pixel_x = result.before_motion.player_x;
    result.observed_pixel_y = result.before_motion.player_y;
    {
        uint32_t left =
            result.before_motion.player_x > 2u ? (uint32_t)result.before_motion.player_x - 2u : 0u;
        uint32_t right = (uint32_t)result.after_motion.player_x + 16u;
        uint32_t top = result.before_motion.player_y < result.after_motion.player_y
                           ? result.before_motion.player_y
                           : result.after_motion.player_y;
        uint32_t bottom = result.before_motion.player_y > result.after_motion.player_y
                              ? result.before_motion.player_y
                              : result.after_motion.player_y;
        if (right > VIDEO_WIDTH)
            right = VIDEO_WIDTH;
        top = top > 2u ? top - 2u : 0u;
        bottom += 18u;
        if (bottom > VIDEO_HEIGHT)
            bottom = VIDEO_HEIGHT;
        result.roi_x = left;
        result.roi_y = top;
        result.roi_width = right - left;
        result.roi_height = bottom - top;
        result.changed_pixels_in_player_sweep =
            count_roi_differences(before_video, after_video, left, top, right - left, bottom - top);
    }
    result.game_state_ok =
        result.gameplay_entry_frame != 0u && result.final_state.attract_mode == 0u &&
        result.final_state.player_enabled == 1u && result.final_state.level_number == 1u;
    result.input_progress_ok =
        result.after_motion.player_x > result.before_motion.player_x &&
        (uint32_t)(result.after_motion.player_x - result.before_motion.player_x) >= 30u;
    result.visible_pixels_ok = result.game_state_ok && result.input_progress_ok &&
                               result.changed_pixels_in_player_sweep > 0u;
    result.gameplay_pcm_ok =
        result.audio_sample_count >= 798u * (AUDIO_LAST_FRAME - AUDIO_FIRST_FRAME + 1u) &&
        result.audio_nonzero_samples > 0u && result.audio_peak_abs > 0;
    result.failure_mask = verdict_failure_mask(&result);

    if (scenario == SCENARIO_POSITIVE) {
        result.buffer_edges_ok = run_buffer_edge_controls(rom, rom_size);
        result.synthetic_controls_ok = run_synthetic_verdict_controls(&result);
        integration_checks_ok = result.refusal_preserved && result.continuation_equal &&
                                result.continuation_frames == REFUSAL_CONTINUATION_FRAMES &&
                                result.continued_player_x > result.before_motion.player_x &&
                                result.buffer_edges_ok && result.synthetic_controls_ok;
    } else {
        /* A semantic negative is a completed emulator run whose expected progress is absent. */
        integration_checks_ok = result.completed_frames == GAMEPLAY_FRAMES &&
                                result.failure_mask != 0u && !result.input_progress_ok;
    }
    if (!write_gameplay_report(report_path, &version, &info, &result, integration_checks_ok)) {
        fprintf(stderr, "could not write gameplay report\n");
        goto done;
    }
    if (scenario == SCENARIO_POSITIVE && !integration_checks_ok)
        fprintf(stderr, "positive gameplay or refusal/buffer/control integrity check failed\n");
    if (scenario != SCENARIO_POSITIVE && (result.completed_frames != GAMEPLAY_FRAMES ||
                                          result.input_progress_ok || result.failure_mask == 0u)) {
        fprintf(stderr,
                "semantic control did not complete with the expected named gameplay rejection\n");
        goto done;
    }
    return_code =
        (scenario == SCENARIO_POSITIVE ? (result.failure_mask == 0u && integration_checks_ok)
                                       : integration_checks_ok)
            ? 0
            : 1;
done:
    nesturbator_destroy(twin);
    nesturbator_destroy(inst);
    free(rom);
    return return_code;
}

int main(int argc, char **argv)
{
    if (argc == 1)
        return run_smoke();
    if (argc == 5 && strcmp(argv[1], "--gameplay") == 0 && strcmp(argv[3], "--report") == 0)
        return run_gameplay_scenario(argv[2], argv[4], SCENARIO_POSITIVE);
    if (argc == 7 && strcmp(argv[1], "--control") == 0 && strcmp(argv[3], "--rom") == 0 &&
        strcmp(argv[5], "--report") == 0) {
        if (strcmp(argv[2], "no-input") == 0)
            return run_gameplay_scenario(argv[4], argv[6], SCENARIO_NO_INPUT);
        if (strcmp(argv[2], "start-only") == 0)
            return run_gameplay_scenario(argv[4], argv[6], SCENARIO_START_ONLY);
    }
    fprintf(stderr, "usage: consumer [--gameplay ROM --report REPORT] "
                    "[--control no-input|start-only --rom ROM --report REPORT]\n");
    return 2;
}
