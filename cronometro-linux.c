#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

extern int64_t monotonic_ns(void);

static struct termios terminal_original;
static int terminal_alterado;
static volatile sig_atomic_t interrompido;

static void restaurar_terminal(void) {
    if (terminal_alterado) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_original);
        terminal_alterado = 0;
    }

    printf("\033[?25h\n");
    fflush(stdout);
}

static void tratar_sigint(int sinal) {
    (void)sinal;
    interrompido = 1;
}

static int preparar_terminal(void) {
    struct termios terminal;
    struct sigaction acao;

    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Execute o cronometro em um terminal interativo.\n");
        return -1;
    }

    if (tcgetattr(STDIN_FILENO, &terminal_original) == -1) {
        perror("tcgetattr");
        return -1;
    }

    terminal = terminal_original;
    terminal.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    terminal.c_cc[VMIN] = 0;
    terminal.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal) == -1) {
        perror("tcsetattr");
        return -1;
    }

    terminal_alterado = 1;

    if (atexit(restaurar_terminal) != 0) {
        restaurar_terminal();
        fprintf(stderr, "Nao foi possivel registrar a restauracao do terminal.\n");
        return -1;
    }

    acao.sa_handler = tratar_sigint;
    sigemptyset(&acao.sa_mask);
    acao.sa_flags = 0;

    if (sigaction(SIGINT, &acao, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    printf("\033[?25l");
    fflush(stdout);
    return 0;
}

static void mostrar_tempo(uint64_t nanossegundos, int rodando) {
    uint64_t centesimos = nanossegundos / 10000000ULL;
    uint64_t horas = centesimos / 360000ULL;
    uint64_t minutos = (centesimos / 6000ULL) % 60ULL;
    uint64_t segundos = (centesimos / 100ULL) % 60ULL;
    uint64_t parte_centessimal = centesimos % 100ULL;

    printf(
        "\r%02" PRIu64 ":%02" PRIu64 ":%02" PRIu64 ".%02" PRIu64
        "   [%s] iniciar/parar  [R] zerar  [Q] sair   ",
        horas,
        minutos,
        segundos,
        parte_centessimal,
        rodando ? "RODANDO" : "PARADO"
    );
    fflush(stdout);
}

int main(void) {
    uint64_t tempo_acumulado = 0;
    uint64_t inicio = 0;
    int rodando = 0;

    if (preparar_terminal() != 0) {
        return 1;
    }

    for (;;) {
        int64_t leitura = monotonic_ns();
        uint64_t agora;
        uint64_t tempo_exibido;
        fd_set entradas;
        struct timeval espera;
        int resultado;

        if (interrompido) {
            break;
        }

        if (leitura < 0) {
            fprintf(stderr, "\nFalha ao ler o relogio monotônico.\n");
            return 1;
        }

        agora = (uint64_t)leitura;
        tempo_exibido = tempo_acumulado;

        if (rodando) {
            tempo_exibido += agora - inicio;
        }

        mostrar_tempo(tempo_exibido, rodando);

        FD_ZERO(&entradas);
        FD_SET(STDIN_FILENO, &entradas);

        espera.tv_sec = 0;
        espera.tv_usec = 50000;

        resultado = select(STDIN_FILENO + 1, &entradas, NULL, NULL, &espera);

        if (interrompido) {
            break;
        }

        if (resultado < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("\nselect");
            return 1;
        }

        if (resultado > 0 && FD_ISSET(STDIN_FILENO, &entradas)) {
            char tecla;
            ssize_t lidos = read(STDIN_FILENO, &tecla, 1);

            if (lidos == 1) {
                int64_t leitura_tecla = monotonic_ns();

                if (leitura_tecla < 0) {
                    fprintf(stderr, "\nFalha ao ler o relogio monotônico.\n");
                    return 1;
                }

                agora = (uint64_t)leitura_tecla;

                if (tecla == 'q' || tecla == 'Q') {
                    break;
                }

                if (tecla == 'r' || tecla == 'R') {
                    tempo_acumulado = 0;
                    if (rodando) {
                        inicio = agora;
                    }
                } else if (tecla == ' ') {
                    if (rodando) {
                        tempo_acumulado += agora - inicio;
                        rodando = 0;
                    } else {
                        inicio = agora;
                        rodando = 1;
                    }
                }
            }
        }
    }

    return 0;
}