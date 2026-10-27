/* Owned, offline GitHub CLI adapter for Phase 20 collector controls. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

#define PHASE20_PATH_CAPACITY 4096

static const char *phase20_env(const char *name)
{
    const char *value = getenv(name);
    return value == NULL ? "" : value;
}

static int phase20_append_call(const char *kind, const char *value)
{
    const char *path = phase20_env("PHASE20_FAKE_GH_CALLS");
    FILE *file;

    if (path[0] == '\0') {
        return 0;
    }
    file = fopen(path, "ab");
    if (file == NULL) {
        (void)fprintf(stderr, "fake gh: cannot append call log (%d)\n", errno);
        return 1;
    }
    (void)fprintf(file, "%s\t%s\n", kind, value);
    if (fclose(file) != 0) {
        (void)fprintf(stderr, "fake gh: cannot close call log\n");
        return 1;
    }
    return 0;
}

static int phase20_copy_stream(FILE *input, FILE *output)
{
    unsigned char buffer[8192];
    size_t count;

    while ((count = fread(buffer, 1U, sizeof buffer, input)) != 0U) {
        if (fwrite(buffer, 1U, count, output) != count) {
            return 1;
        }
    }
    return ferror(input) ? 1 : 0;
}

static int phase20_serve_file(const char *fixture, const char *relative_path)
{
    char full_path[PHASE20_PATH_CAPACITY];
    FILE *input;
    int result;
    int path_length;

    path_length = snprintf(full_path, sizeof full_path, "%s/%s", fixture, relative_path);
    if (path_length < 0 || (size_t)path_length >= sizeof full_path) {
        (void)fprintf(stderr, "fake gh: fixture path is too long\n");
        return 1;
    }
    input = fopen(full_path, "rb");
    if (input == NULL) {
        (void)fprintf(stderr, "fake gh: fixture response is missing\n");
        return 1;
    }
    result = phase20_copy_stream(input, stdout);
    if (fclose(input) != 0) {
        result = 1;
    }
    return result;
}

static int phase20_match_route(const char *fixture, const char *endpoint)
{
    char routes_path[PHASE20_PATH_CAPACITY];
    char line[PHASE20_PATH_CAPACITY];
    FILE *routes;
    int path_length;

    path_length = snprintf(routes_path, sizeof routes_path, "%s/routes.tsv", fixture);
    if (path_length < 0 || (size_t)path_length >= sizeof routes_path) {
        (void)fprintf(stderr, "fake gh: route path is too long\n");
        return 1;
    }
    routes = fopen(routes_path, "rb");
    if (routes == NULL) {
        (void)fprintf(stderr, "fake gh: route table is missing\n");
        return 1;
    }
    while (fgets(line, (int)sizeof line, routes) != NULL) {
        char *separator;
        size_t length = strlen(line);

        if (length > 0U && line[length - 1U] == '\n') {
            line[--length] = '\0';
        }
        separator = strchr(line, '\t');
        if (separator == NULL) {
            (void)fclose(routes);
            (void)fprintf(stderr, "fake gh: malformed route table\n");
            return 1;
        }
        *separator++ = '\0';
        if (strcmp(line, endpoint) == 0) {
            int result = phase20_serve_file(fixture, separator);
            (void)fclose(routes);
            return result;
        }
    }
    (void)fclose(routes);
    (void)fprintf(stderr, "fake gh: unexpected API endpoint\n");
    return 1;
}

static int phase20_failure_for_endpoint(const char *endpoint)
{
    const char *failure = phase20_env("PHASE20_FAKE_GH_FAILURE");
    const char *match = phase20_env("PHASE20_FAKE_GH_FAIL_MATCH");

    if (failure[0] == '\0' || (match[0] != '\0' && strstr(endpoint, match) == NULL)) {
        return 0;
    }
    if (strcmp(failure, "auth") == 0) {
        (void)fprintf(stderr, "gh: not logged in to github.com\n");
    } else if (strcmp(failure, "cli") == 0) {
        (void)fprintf(stderr, "gh: unknown flag: --repo for api\n");
    } else {
        (void)fprintf(stderr, "gh: HTTP 502: fixture API unavailable\n");
    }
    return 1;
}

static int phase20_handle_api(int argc, char **argv)
{
    const char *fixture = phase20_env("PHASE20_FAKE_GH_FIXTURE");
    const char *endpoint = NULL;
    int index;

    for (index = 2; index < argc; ++index) {
        if (strcmp(argv[index], "--repo") == 0) {
            (void)phase20_append_call("rejected-api-flag", "--repo");
            (void)fprintf(stderr, "gh: unknown flag: --repo for api\n");
            return 64;
        }
        if (argv[index][0] != '-') {
            if (endpoint != NULL) {
                (void)fprintf(stderr, "fake gh: unexpected extra API argument\n");
                return 64;
            }
            endpoint = argv[index];
        }
    }
    if (endpoint == NULL || fixture[0] == '\0') {
        (void)fprintf(stderr, "fake gh: api endpoint or fixture root is missing\n");
        return 64;
    }
    if (phase20_append_call("api", endpoint) != 0) {
        return 65;
    }
    if (phase20_env("PHASE20_FAKE_GH_CONTENT_FORBIDDEN")[0] == '1' &&
        (strstr(endpoint, "/logs") != NULL || strstr(endpoint, "/zip") != NULL)) {
        (void)fprintf(stderr, "fake gh: availability mode requested content endpoint\n");
        return 66;
    }
    if (phase20_failure_for_endpoint(endpoint)) {
        return 67;
    }
    return phase20_match_route(fixture, endpoint);
}

static int phase20_handle_run_view(int argc, char **argv)
{
    const char *fixture = phase20_env("PHASE20_FAKE_GH_FIXTURE");
    const char *run_id = NULL;
    const char *attempt = NULL;
    char route[128];
    int index;

    if (argc < 4 || fixture[0] == '\0') {
        (void)fprintf(stderr, "fake gh: malformed run-view command\n");
        return 64;
    }
    run_id = argv[3];
    for (index = 4; index + 1 < argc; ++index) {
        if (strcmp(argv[index], "--attempt") == 0) {
            attempt = argv[index + 1];
        }
    }
    if (attempt == NULL || snprintf(route, sizeof route, "%s/%s", run_id, attempt) < 0) {
        (void)fprintf(stderr, "fake gh: run-view identity is incomplete\n");
        return 64;
    }
    if (phase20_env("PHASE20_FAKE_GH_CONTENT_FORBIDDEN")[0] == '1') {
        (void)phase20_append_call("rejected-content", "workflow-run-log");
        (void)fprintf(stderr, "fake gh: availability mode requested workflow logs\n");
        return 66;
    }
    if (phase20_append_call("run-view", route) != 0) {
        return 65;
    }
    return phase20_match_route(fixture, route);
}

int main(int argc, char **argv)
{
#if defined(_WIN32)
    (void)_setmode(_fileno(stdout), _O_BINARY);
#endif
    if (argc < 2) {
        (void)fprintf(stderr, "fake gh: expected api or run command\n");
        return 64;
    }
    if (strcmp(argv[1], "api") == 0) {
        return phase20_handle_api(argc, argv);
    }
    if (strcmp(argv[1], "run") == 0 && argc >= 3 && strcmp(argv[2], "view") == 0) {
        return phase20_handle_run_view(argc, argv);
    }
    (void)fprintf(stderr, "fake gh: unexpected gh command\n");
    return 64;
}
