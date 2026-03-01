#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#define SHM_NAME "/ipc_shm"
#define SOCK_PATH "/tmp/ipc_sock"
#define DATA_SIZE 64

typedef struct {
    int pos;
    char data[DATA_SIZE];
} Shared;

/* enviar file descriptor */
int send_fd(int sock, int fd) {
    struct msghdr msg = {0};
    char buf[CMSG_SPACE(sizeof(int))];
    memset(buf, 0, sizeof(buf));

    struct iovec io = { .iov_base = (void *)"X", .iov_len = 1 };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    *((int *) CMSG_DATA(cmsg)) = fd;

    return sendmsg(sock, &msg, 0);
}

/* recibir file descriptor */
int recv_fd(int sock) {
    struct msghdr msg = {0};
    char m[1];
    char buf[CMSG_SPACE(sizeof(int))];
    struct iovec io = { .iov_base = m, .iov_len = sizeof(m) };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    if (recvmsg(sock, &msg, 0) < 0) return -1;

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg) return -1;

    return *((int *) CMSG_DATA(cmsg));
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <n> <x>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    char x = argv[2][0];

    if (n <= 0) {
        fprintf(stderr, "n debe ser mayor que 0\n");
        return 1;
    }

    printf("I am %c\n", x);

    int shm_fd, recv_shm_fd = -1;
    int created = 0;
    Shared *ptr;

    /* intentar crear la memoria compartida */
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);

    if (shm_fd >= 0) {
        created = 1;
        printf("%c: Created new shared mem obj %d\n", x, shm_fd);

        if (ftruncate(shm_fd, sizeof(Shared)) < 0) {
            perror("ftruncate");
            return 1;
        }

        ptr = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap");
            return 1;
        }

        printf("%c: Ptr created with value %p\n", x, (void *)ptr);

        ptr->pos = 0;
        memset(ptr->data, 0, sizeof(ptr->data));
        printf("%c: Initialized shared mem obj\n", x);

        /* preparar socket para mandar el fd */
        int sfd, cfd;
        struct sockaddr_un addr;

        unlink(SOCK_PATH);

        sfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sfd < 0) {
            perror("socket");
            return 1;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

        if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind");
            return 1;
        }

        if (listen(sfd, 1) < 0) {
            perror("listen");
            return 1;
        }

        cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            perror("accept");
            return 1;
        }

        if (send_fd(cfd, shm_fd) < 0) {
            perror("send_fd");
            return 1;
        }

        close(cfd);
        close(sfd);
        unlink(SOCK_PATH);

    } else {
        if (errno != EEXIST) {
            perror("shm_open");
            return 1;
        }

        printf("%c: Shared mem obj already created\n", x);

        /* abrirla normalmente para mapear */
        shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (shm_fd < 0) {
            perror("shm_open existing");
            return 1;
        }

        /* conectarse al socket y recibir el fd */
        int sock;
        struct sockaddr_un addr;

        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("socket");
            return 1;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

        for (int i = 0; i < 20; i++) {
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
                recv_shm_fd = recv_fd(sock);
                break;
            }
            usleep(100000);
        }

        printf("%c: Received shm fd: %d\n", x, recv_shm_fd);
        close(sock);

        /* NO usar recv_shm_fd para mmap */
        ptr = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap");
            return 1;
        }

        printf("%c: Ptr created with value %p\n", x, (void *)ptr);

        if (recv_shm_fd >= 0) close(recv_shm_fd);
    }

    /* pipe padre -> hijo */
    int p[2];
    if (pipe(p) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* hijo: recibe x y escribe */
        close(p[1]);

        char ch;
        while (read(p[0], &ch, 1) > 0) {
            int idx = __sync_fetch_and_add(&ptr->pos, 1);
            if (idx < DATA_SIZE - 1) {
                ptr->data[idx] = ch;
                ptr->data[idx + 1] = '\0';
            }
        }

        close(p[0]);
        munmap(ptr, sizeof(Shared));
        close(shm_fd);
        exit(0);
    } else {
        /* padre: recorre y notifica al hijo */
        close(p[0]);

        for (int i = 0; i < DATA_SIZE; i++) {
            if (i % n == 0) {
                write(p[1], &x, 1);
            }
        }

        close(p[1]);
        wait(NULL);

        printf("%c: Shared memory has: %s\n", x, ptr->data);

        munmap(ptr, sizeof(Shared));
        close(shm_fd);

        if (created) {
            shm_unlink(SHM_NAME);
        }
    }

    return 0;
}