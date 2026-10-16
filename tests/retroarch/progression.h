#ifndef NESTURBATOR_TEST_PROGRESSION_H
#define NESTURBATOR_TEST_PROGRESSION_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRESSION_REQUIRED_BOUNDARIES 16u
#define PROGRESSION_MAX_BOUNDARIES 32u
#define PROGRESSION_PATH_CAP 2048u
#define PROGRESSION_RECORD_CAP 65536u
#define PROGRESSION_ARTIFACT_CAP (1024u * 1024u)
#define PROGRESSION_FRAME_FIRST 4u
#define PROGRESSION_FRAME_LAST 10000u
#define PROGRESSION_RUNAHEAD_DISPLAY_LEAD 1

typedef enum { PROGRESSION_PREFIX = 1, PROGRESSION_COMPLETE = 2 } progression_mode;

typedef enum {
    PROGRESSION_CAPTURE = 1,
    PROGRESSION_ADVANCE,
    PROGRESSION_LOAD_PROBE,
    PROGRESSION_RESUME_AND_PAUSE,
    PROGRESSION_REWIND_BURST,
    PROGRESSION_RUNAHEAD_LOAD_PROBE
} progression_operation;

typedef enum {
    PROGRESSION_PAUSE_EQUAL_ORIGINAL = 1,
    PROGRESSION_ORIGINAL_PLUS,
    PROGRESSION_ORIGINAL_LOAD_PROBE,
    PROGRESSION_ORIGINAL_POST_LOAD,
    PROGRESSION_RESUME_FORWARD,
    PROGRESSION_REWIND_COARSE,
    PROGRESSION_REWOUND_PLUS,
    PROGRESSION_REWOUND_LOAD_PROBE
} progression_relation;

typedef enum {
    PROGRESSION_ORIGIN_NONE = 0,
    PROGRESSION_ORIGIN_ORIGINAL,
    PROGRESSION_ORIGIN_REWOUND
} progression_origin;

typedef struct {
    uint32_t id;
    progression_operation operation;
    progression_relation relation;
    progression_origin origin;
    uint32_t command_count;
    uint32_t send_count;
    uint64_t artifact_identity;
    char artifact_path[PROGRESSION_PATH_CAP];
    uint64_t artifact_hash;
    uint64_t display_frame;
    uint32_t candidate_count;
    uint64_t candidate_first;
    uint64_t candidate_last;
    uint64_t anchor_identity;
    uint64_t anchor_frame;
    int32_t display_lead;
    uint32_t expected_delta;
    uint64_t elapsed_ms;
    uint64_t deadline_ms;
    int process_alive;
} progression_boundary;

typedef struct {
    progression_mode mode;
    uint64_t content_identity;
    uint64_t calibration_identity;
    int terminal_exit; /* -1 means the observed host is still running. */
    char original_anchor_path[PROGRESSION_PATH_CAP];
    uint64_t original_anchor_identity;
    uint64_t original_anchor_frame;
    char rewound_anchor_path[PROGRESSION_PATH_CAP];
    uint64_t rewound_anchor_identity;
    uint64_t rewound_anchor_frame;
    uint32_t boundary_count;
    progression_boundary boundaries[PROGRESSION_MAX_BOUNDARIES];
} progression_session;

static inline const char *progression_operation_name(progression_operation operation)
{
    switch (operation) {
    case PROGRESSION_CAPTURE:
        return "capture";
    case PROGRESSION_ADVANCE:
        return "advance";
    case PROGRESSION_LOAD_PROBE:
        return "load-probe";
    case PROGRESSION_RESUME_AND_PAUSE:
        return "resume-and-pause";
    case PROGRESSION_REWIND_BURST:
        return "rewind-burst";
    case PROGRESSION_RUNAHEAD_LOAD_PROBE:
        return "runahead-load-probe";
    }
    return NULL;
}

static inline const char *progression_relation_name(progression_relation relation)
{
    switch (relation) {
    case PROGRESSION_PAUSE_EQUAL_ORIGINAL:
        return "pause-equal-original";
    case PROGRESSION_ORIGINAL_PLUS:
        return "original-plus";
    case PROGRESSION_ORIGINAL_LOAD_PROBE:
        return "original-load-probe";
    case PROGRESSION_ORIGINAL_POST_LOAD:
        return "original-post-load";
    case PROGRESSION_RESUME_FORWARD:
        return "resume-forward";
    case PROGRESSION_REWIND_COARSE:
        return "rewind-coarse";
    case PROGRESSION_REWOUND_PLUS:
        return "rewound-plus";
    case PROGRESSION_REWOUND_LOAD_PROBE:
        return "rewound-load-probe";
    }
    return NULL;
}

static inline const char *progression_origin_name(progression_origin origin)
{
    switch (origin) {
    case PROGRESSION_ORIGIN_NONE:
        return "none";
    case PROGRESSION_ORIGIN_ORIGINAL:
        return "original";
    case PROGRESSION_ORIGIN_REWOUND:
        return "rewound";
    }
    return NULL;
}

static inline uint64_t progression_hash64(const void *bytes, size_t size)
{
    const uint8_t *p = (const uint8_t *)bytes;
    uint64_t hash = UINT64_C(14695981039346656037);
    for (size_t i = 0; i < size; ++i) {
        hash ^= p[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static inline int progression_hash_file(const char *path, uint64_t *hash_out)
{
    uint8_t buffer[4096];
    uint64_t hash = UINT64_C(14695981039346656037);
    size_t total = 0;
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return 0;
    for (;;) {
        size_t count = fread(buffer, 1u, sizeof buffer, file);
        if (count > PROGRESSION_ARTIFACT_CAP - total) {
            fclose(file);
            return 0;
        }
        total += count;
        for (size_t i = 0; i < count; ++i) {
            hash ^= buffer[i];
            hash *= UINT64_C(1099511628211);
        }
        if (count < sizeof buffer) {
            if (ferror(file)) {
                fclose(file);
                return 0;
            }
            break;
        }
    }
    if (fclose(file) != 0 || total == 0u)
        return 0;
    *hash_out = hash;
    return 1;
}

static inline int progression_line(const uint8_t *data, size_t size, size_t *cursor,
                                   const uint8_t **line, size_t *line_size)
{
    size_t start = *cursor;
    size_t end = start;
    while (end < size && data[end] != (uint8_t)'\n') {
        if (data[end] == 0u || data[end] == (uint8_t)'\r')
            return 0;
        ++end;
    }
    if (end == size)
        return 0;
    *line = data + start;
    *line_size = end - start;
    *cursor = end + 1u;
    return 1;
}

static inline int progression_expect_line(const uint8_t *data, size_t size, size_t *cursor,
                                          const char *expected)
{
    const uint8_t *line;
    size_t line_size;
    size_t expected_size = strlen(expected);
    return progression_line(data, size, cursor, &line, &line_size) && line_size == expected_size &&
           memcmp(line, expected, expected_size) == 0;
}

static inline int progression_take_value(const uint8_t *data, size_t size, size_t *cursor,
                                         const char *key, char *value, size_t capacity,
                                         size_t *value_size)
{
    const uint8_t *line;
    size_t line_size;
    size_t key_size = strlen(key);
    if (!progression_line(data, size, cursor, &line, &line_size) || line_size < key_size + 1u ||
        memcmp(line, key, key_size) != 0 || line[key_size] != '=')
        return 0;
    size_t n = line_size - key_size - 1u;
    if (n >= capacity)
        return 0;
    memcpy(value, line + key_size + 1u, n);
    value[n] = '\0';
    if (value_size != NULL)
        *value_size = n;
    return 1;
}

static inline int progression_decimal(const char *text, uint64_t *value)
{
    uint64_t n = 0;
    if (*text == '\0')
        return 0;
    for (const unsigned char *p = (const unsigned char *)text; *p != 0u; ++p) {
        if (*p < (unsigned char)'0' || *p > (unsigned char)'9')
            return 0;
        uint64_t digit = (uint64_t)(*p - (unsigned char)'0');
        if (n > (UINT64_MAX - digit) / 10u)
            return 0;
        n = n * 10u + digit;
    }
    *value = n;
    return 1;
}

static inline int progression_hex64(const char *text, uint64_t *value)
{
    uint64_t n = 0;
    if (strlen(text) != 16u)
        return 0;
    for (const unsigned char *p = (const unsigned char *)text; *p != 0u; ++p) {
        unsigned digit;
        if (*p >= (unsigned char)'0' && *p <= (unsigned char)'9')
            digit = (unsigned)(*p - (unsigned char)'0');
        else if (*p >= (unsigned char)'a' && *p <= (unsigned char)'f')
            digit = 10u + (unsigned)(*p - (unsigned char)'a');
        else
            return 0;
        n = (n << 4) | digit;
    }
    *value = n;
    return 1;
}

static inline int progression_take_u64(const uint8_t *data, size_t size, size_t *cursor,
                                       const char *key, uint64_t *value)
{
    char text[32];
    return progression_take_value(data, size, cursor, key, text, sizeof text, NULL) &&
           progression_decimal(text, value);
}

static inline int progression_take_hex(const uint8_t *data, size_t size, size_t *cursor,
                                       const char *key, uint64_t *value)
{
    char text[32];
    return progression_take_value(data, size, cursor, key, text, sizeof text, NULL) &&
           progression_hex64(text, value);
}

static inline int progression_take_path(const uint8_t *data, size_t size, size_t *cursor,
                                        const char *length_key, const char *path_key, char *path)
{
    uint64_t path_size;
    size_t actual_size;
    return progression_take_u64(data, size, cursor, length_key, &path_size) &&
           path_size < PROGRESSION_PATH_CAP &&
           progression_take_value(data, size, cursor, path_key, path, PROGRESSION_PATH_CAP,
                                  &actual_size) &&
           path_size == actual_size;
}

static inline int progression_parse_operation(const char *text, progression_operation *operation)
{
    for (progression_operation i = PROGRESSION_CAPTURE; i <= PROGRESSION_RUNAHEAD_LOAD_PROBE;
         i = (progression_operation)(i + 1)) {
        const char *name = progression_operation_name(i);
        if (name != NULL && strcmp(text, name) == 0) {
            *operation = i;
            return 1;
        }
    }
    return 0;
}

static inline int progression_parse_relation(const char *text, progression_relation *relation)
{
    for (progression_relation i = PROGRESSION_PAUSE_EQUAL_ORIGINAL;
         i <= PROGRESSION_REWOUND_LOAD_PROBE; i = (progression_relation)(i + 1)) {
        const char *name = progression_relation_name(i);
        if (name != NULL && strcmp(text, name) == 0) {
            *relation = i;
            return 1;
        }
    }
    return 0;
}

static inline int progression_parse_origin(const char *text, progression_origin *origin)
{
    for (progression_origin i = PROGRESSION_ORIGIN_NONE; i <= PROGRESSION_ORIGIN_REWOUND;
         i = (progression_origin)(i + 1)) {
        const char *name = progression_origin_name(i);
        if (name != NULL && strcmp(text, name) == 0) {
            *origin = i;
            return 1;
        }
    }
    return 0;
}

static inline const char *progression_parse_record(const uint8_t *data, size_t size,
                                                   progression_session *session)
{
    size_t cursor = 0;
    char text[PROGRESSION_PATH_CAP];
    uint64_t number;
    if (size == 0u || size > PROGRESSION_RECORD_CAP || session == NULL)
        return "PARSE";
    memset(session, 0, sizeof *session);
    session->terminal_exit = -1;
    if (!progression_expect_line(data, size, &cursor, "NESTURBATOR-PROGRESSION 1"))
        return "PARSE";
    if (!progression_take_value(data, size, &cursor, "mode", text, sizeof text, NULL))
        return "PARSE";
    if (strcmp(text, "prefix") == 0)
        session->mode = PROGRESSION_PREFIX;
    else if (strcmp(text, "complete") == 0)
        session->mode = PROGRESSION_COMPLETE;
    else
        return "PARSE";
    if (!progression_take_hex(data, size, &cursor, "content_identity",
                              &session->content_identity) ||
        !progression_take_hex(data, size, &cursor, "calibration_identity",
                              &session->calibration_identity) ||
        !progression_take_value(data, size, &cursor, "terminal_exit", text, sizeof text, NULL))
        return "PARSE";
    if (strcmp(text, "running") != 0) {
        if (!progression_decimal(text, &number) || number > 255u)
            return "PARSE";
        session->terminal_exit = (int)number;
    }
    if (!progression_take_path(data, size, &cursor, "original_anchor_path_length",
                               "original_anchor_path", session->original_anchor_path) ||
        !progression_take_hex(data, size, &cursor, "original_anchor_identity",
                              &session->original_anchor_identity) ||
        !progression_take_u64(data, size, &cursor, "original_anchor_frame",
                              &session->original_anchor_frame) ||
        !progression_take_path(data, size, &cursor, "rewound_anchor_path_length",
                               "rewound_anchor_path", session->rewound_anchor_path) ||
        !progression_take_hex(data, size, &cursor, "rewound_anchor_identity",
                              &session->rewound_anchor_identity) ||
        !progression_take_u64(data, size, &cursor, "rewound_anchor_frame",
                              &session->rewound_anchor_frame) ||
        !progression_take_u64(data, size, &cursor, "boundary_count", &number) ||
        number > PROGRESSION_MAX_BOUNDARIES)
        return "PARSE";
    session->boundary_count = (uint32_t)number;
    for (uint32_t i = 0; i < session->boundary_count; ++i) {
        progression_boundary *b = &session->boundaries[i];
        if (!progression_expect_line(data, size, &cursor, "boundary") ||
            !progression_take_u64(data, size, &cursor, "id", &number) || number > UINT32_MAX)
            return "PARSE";
        b->id = (uint32_t)number;
        if (!progression_take_value(data, size, &cursor, "operation", text, sizeof text, NULL) ||
            !progression_parse_operation(text, &b->operation) ||
            !progression_take_value(data, size, &cursor, "relation", text, sizeof text, NULL) ||
            !progression_parse_relation(text, &b->relation) ||
            !progression_take_value(data, size, &cursor, "origin", text, sizeof text, NULL) ||
            !progression_parse_origin(text, &b->origin) ||
            !progression_take_u64(data, size, &cursor, "command_count", &number) ||
            number > UINT32_MAX)
            return "PARSE";
        b->command_count = (uint32_t)number;
        if (!progression_take_u64(data, size, &cursor, "send_count", &number) ||
            number > UINT32_MAX)
            return "PARSE";
        b->send_count = (uint32_t)number;
        if (!progression_take_u64(data, size, &cursor, "artifact_identity",
                                  &b->artifact_identity) ||
            !progression_take_path(data, size, &cursor, "artifact_path_length", "artifact_path",
                                   b->artifact_path) ||
            !progression_take_hex(data, size, &cursor, "artifact_hash", &b->artifact_hash) ||
            !progression_take_u64(data, size, &cursor, "display_frame", &b->display_frame) ||
            !progression_take_u64(data, size, &cursor, "candidate_count", &number) ||
            number > UINT32_MAX)
            return "PARSE";
        b->candidate_count = (uint32_t)number;
        if (!progression_take_u64(data, size, &cursor, "candidate_first", &b->candidate_first) ||
            !progression_take_u64(data, size, &cursor, "candidate_last", &b->candidate_last) ||
            !progression_take_hex(data, size, &cursor, "anchor_identity", &b->anchor_identity) ||
            !progression_take_u64(data, size, &cursor, "anchor_frame", &b->anchor_frame) ||
            !progression_take_u64(data, size, &cursor, "display_lead", &number) ||
            number > INT32_MAX)
            return "PARSE";
        b->display_lead = (int32_t)number;
        if (!progression_take_u64(data, size, &cursor, "expected_delta", &number) ||
            number > UINT32_MAX)
            return "PARSE";
        b->expected_delta = (uint32_t)number;
        if (!progression_take_u64(data, size, &cursor, "elapsed_ms", &b->elapsed_ms) ||
            !progression_take_u64(data, size, &cursor, "deadline_ms", &b->deadline_ms) ||
            !progression_take_u64(data, size, &cursor, "process_alive", &number) || number > 1u)
            return "PARSE";
        b->process_alive = (int)number;
        if (!progression_expect_line(data, size, &cursor, "boundary_end"))
            return "PARSE";
    }
    if (!progression_expect_line(data, size, &cursor, "END") || cursor != size)
        return "PARSE";
    return NULL;
}

static inline const char *progression_read_record(const char *path, progression_session *session)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return "IO";
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return "IO";
    }
    long length = ftell(file);
    if (length <= 0 || (unsigned long)length > PROGRESSION_RECORD_CAP ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return "PARSE";
    }
    size_t size = (size_t)length;
    uint8_t *data = (uint8_t *)malloc(size);
    if (data == NULL) {
        fclose(file);
        return "IO";
    }
    size_t read_size = fread(data, 1u, size, file);
    int read_ok = read_size == size && !ferror(file);
    if (fclose(file) != 0)
        read_ok = 0;
    const char *reason = read_ok ? progression_parse_record(data, size, session) : "IO";
    free(data);
    return reason;
}

static inline void progression_expected(uint32_t index, progression_operation *operation,
                                        progression_relation *relation, progression_origin *origin,
                                        uint32_t *commands, uint32_t *sends, uint32_t *delta,
                                        int32_t *lead)
{
    *commands = 1u;
    *sends = 1u;
    *delta = 0u;
    *lead = 0;
    *origin = PROGRESSION_ORIGIN_ORIGINAL;
    if (index <= 1u) {
        *operation = PROGRESSION_CAPTURE;
        *relation = PROGRESSION_PAUSE_EQUAL_ORIGINAL;
    } else if (index <= 6u) {
        *operation = PROGRESSION_ADVANCE;
        *relation = PROGRESSION_ORIGINAL_PLUS;
        *delta = index - 1u;
    } else if (index == 7u) {
        *operation = PROGRESSION_LOAD_PROBE;
        *relation = PROGRESSION_ORIGINAL_LOAD_PROBE;
        *commands = 2u;
        *sends = 2u;
        *delta = 1u;
    } else if (index <= 9u) {
        *operation = PROGRESSION_ADVANCE;
        *relation = PROGRESSION_ORIGINAL_POST_LOAD;
        *delta = index - 6u;
    } else if (index == 10u) {
        *operation = PROGRESSION_RESUME_AND_PAUSE;
        *relation = PROGRESSION_RESUME_FORWARD;
        *commands = 2u;
        *sends = 2u;
    } else if (index == 11u) {
        *operation = PROGRESSION_REWIND_BURST;
        *relation = PROGRESSION_REWIND_COARSE;
        *origin = PROGRESSION_ORIGIN_REWOUND;
        *commands = 40u;
        *sends = 40u;
    } else if (index <= 14u) {
        *operation = PROGRESSION_ADVANCE;
        *relation = PROGRESSION_REWOUND_PLUS;
        *origin = PROGRESSION_ORIGIN_REWOUND;
        *delta = index - 11u;
    } else {
        *operation = PROGRESSION_RUNAHEAD_LOAD_PROBE;
        *relation = PROGRESSION_REWOUND_LOAD_PROBE;
        *origin = PROGRESSION_ORIGIN_REWOUND;
        *commands = 2u;
        *sends = 2u;
        *delta = 1u;
        *lead = PROGRESSION_RUNAHEAD_DISPLAY_LEAD;
    }
}

static inline const char *progression_verdict(const progression_session *session,
                                              int require_complete)
{
    if (session == NULL || session->boundary_count == 0u)
        return "INCOMPLETE";
    if (session->boundary_count > PROGRESSION_REQUIRED_BOUNDARIES)
        return "ATTRIBUTION";
    if (session->content_identity == 0u || session->calibration_identity == 0u)
        return "IDENTITY";
    if (session->original_anchor_identity == 0u || session->original_anchor_path[0] == '\0' ||
        session->original_anchor_frame < PROGRESSION_FRAME_FIRST ||
        session->original_anchor_frame >= PROGRESSION_FRAME_LAST)
        return "ORIGIN";
    if (session->boundary_count > 11u &&
        (session->rewound_anchor_identity == 0u || session->rewound_anchor_path[0] == '\0' ||
         session->rewound_anchor_frame < PROGRESSION_FRAME_FIRST ||
         session->rewound_anchor_frame >= PROGRESSION_FRAME_LAST))
        return "ORIGIN";

    for (uint32_t i = 0; i < session->boundary_count; ++i) {
        const progression_boundary *b = &session->boundaries[i];
        progression_operation operation;
        progression_relation relation;
        progression_origin origin;
        uint32_t commands, sends, delta;
        int32_t lead;
        progression_expected(i, &operation, &relation, &origin, &commands, &sends, &delta, &lead);
        if (b->id != i + 1u || b->artifact_identity == 0u || b->artifact_hash == 0u ||
            b->artifact_path[0] == '\0')
            return "ATTRIBUTION";
        if (b->candidate_count > 1u)
            return "AMBIGUOUS";
        if (b->candidate_count == 0u || b->candidate_first != b->display_frame ||
            b->candidate_last != b->display_frame)
            return "IDENTITY";
        if (b->display_frame < PROGRESSION_FRAME_FIRST || b->display_frame > PROGRESSION_FRAME_LAST)
            return "IDENTITY";
        if (b->deadline_ms == 0u || b->elapsed_ms > b->deadline_ms)
            return "DEADLINE";
        if (!b->process_alive)
            return "PROCESS";
        if (b->operation != operation || b->relation != relation || b->origin != origin ||
            b->command_count != commands || b->send_count != sends || b->expected_delta != delta ||
            b->display_lead != lead)
            return "PROGRESSION";
        for (uint32_t j = 0; j < i; ++j) {
            const progression_boundary *prior = &session->boundaries[j];
            if (prior->artifact_identity == b->artifact_identity ||
                strcmp(prior->artifact_path, b->artifact_path) == 0)
                return "ATTRIBUTION";
        }
        if (origin == PROGRESSION_ORIGIN_ORIGINAL) {
            if (b->anchor_identity != session->original_anchor_identity ||
                b->anchor_frame != session->original_anchor_frame)
                return "ORIGIN";
        } else if (origin == PROGRESSION_ORIGIN_REWOUND) {
            if (b->anchor_identity != session->rewound_anchor_identity ||
                b->anchor_frame != session->rewound_anchor_frame)
                return "ORIGIN";
        } else
            return "ORIGIN";

        uint64_t original = session->original_anchor_frame;
        uint64_t rewound = session->rewound_anchor_frame;
        switch (relation) {
        case PROGRESSION_PAUSE_EQUAL_ORIGINAL:
            if (b->display_frame != original)
                return "PROGRESSION";
            if (i == 1u && b->display_frame != session->boundaries[0].display_frame)
                return "PROGRESSION";
            break;
        case PROGRESSION_ORIGINAL_PLUS:
        case PROGRESSION_ORIGINAL_POST_LOAD:
            if (b->display_frame != original + delta)
                return "PROGRESSION";
            break;
        case PROGRESSION_ORIGINAL_LOAD_PROBE:
            if (b->display_frame != original + 1u)
                return "PROGRESSION";
            break;
        case PROGRESSION_RESUME_FORWARD:
            if (b->display_frame <= original + 3u)
                return "PROGRESSION";
            break;
        case PROGRESSION_REWIND_COARSE:
            if (i != 11u || b->command_count != 40u || b->send_count != 40u ||
                session->boundaries[10].display_frame < 20u ||
                b->display_frame > session->boundaries[10].display_frame - 20u ||
                b->display_frame != rewound)
                return "PROGRESSION";
            break;
        case PROGRESSION_REWOUND_PLUS:
            if (b->display_frame != rewound + delta)
                return "PROGRESSION";
            break;
        case PROGRESSION_REWOUND_LOAD_PROBE:
            if (b->display_frame != rewound + 1u + (uint64_t)lead)
                return "PROGRESSION";
            break;
        }
    }
    if (session->terminal_exit > 0)
        return "PROCESS";
    if (require_complete) {
        if (session->mode != PROGRESSION_COMPLETE ||
            session->boundary_count != PROGRESSION_REQUIRED_BOUNDARIES ||
            session->terminal_exit < 0)
            return "INCOMPLETE";
        if (session->terminal_exit != 0)
            return "PROCESS";
        return "VERDICT_OK";
    }
    if (session->mode != PROGRESSION_PREFIX || session->terminal_exit >= 0)
        return "INCOMPLETE";
    return "PREFIX_OK";
}

#endif
