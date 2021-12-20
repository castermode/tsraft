#include "aio.h"

#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "munit.h"

int AioFill(aio_context_t *ctx, unsigned n)
{
    char buf[256];
    int fd;
    int rv;
    int limit;
    int used;

    /* Figure out how many events are available. */
    fd = open("/proc/sys/fs/aio-max-nr", O_RDONLY);
    munit_assert_int(fd, !=, -1);

    rv = read(fd, buf, sizeof buf);
    munit_assert_int(rv, !=, -1);

    close(fd);

    limit = atoi(buf);

    /* Figure out how many events are in use. */
    fd = open("/proc/sys/fs/aio-nr", O_RDONLY);
    munit_assert_int(fd, !=, -1);

    rv = read(fd, buf, sizeof buf);
    munit_assert_int(rv, !=, -1);

    close(fd);

    used = atoi(buf);

    /* Best effort check that nothing process is using AIO. Our own unit tests
     * case use up to 2 event slots at the time this function is called, so we
     * don't consider those. */
    if (used > 2) {
        return -1;
    }

    rv = syscall(__NR_io_setup, limit - used - n, ctx);
    munit_assert_int(rv, ==, 0);

    return 0;
}

void AioDestroy(aio_context_t ctx)
{
    int rv;

    rv = syscall(__NR_io_destroy, ctx);
    munit_assert_int(rv, ==, 0);
}
