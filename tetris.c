// tetris.c
// Desafio Tetris Stack - Nível MESTRE (inclui Novato + Aventureiro)
// Fila circular (5) + Pilha (3) com operações de jogar, reservar, usar reserva
// e trocas (frente<->topo e 3 primeiros <-> 3 da pilha).

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// -----------------------------
// Definições gerais
// -----------------------------
#define CAP_FILA   5
#define CAP_PILHA  3

typedef struct {
    char tipo;   // 'I', 'O', 'T', 'L', 'J', 'S', 'Z'
    int  id;     // sequencial
} Peca;

// -----------------------------
// Fila circular de peças futuras
// -----------------------------
typedef struct {
    Peca dados[CAP_FILA];
    int frente;       // índice do primeiro
    int tamanho;      // quantidade de elementos
} Fila;

static inline bool filaVazia(Fila *f)    { return f->tamanho == 0; }
static inline bool filaCheia(Fila *f)    { return f->tamanho == CAP_FILA; }

void inicializarFila(Fila *f) {
    f->frente = 0;
    f->tamanho = 0;
}

int idxCircular(Fila *f, int k) {
    // retorna índice do k-ésimo a partir da frente
    return (f->frente + k) % CAP_FILA;
}

bool enqueue(Fila *f, Peca p) {
    if (filaCheia(f)) return false;
    int pos = idxCircular(f, f->tamanho);
    f->dados[pos] = p;
    f->tamanho++;
    return true;
}

bool dequeue(Fila *f, Peca *out) {
    if (filaVazia(f)) return false;
    if (out) *out = f->dados[f->frente];
    f->frente = (f->frente + 1) % CAP_FILA;
    f->tamanho--;
    return true;
}

// -----------------------------
// Pilha de reserva
// -----------------------------
typedef struct {
    Peca dados[CAP_PILHA];
    int topo;    // quantidade; topo está em topo-1
} Pilha;

static inline bool pilhaVazia(Pilha *p)  { return p->topo == 0; }
static inline bool pilhaCheia(Pilha *p)  { return p->topo == CAP_PILHA; }

void inicializarPilha(Pilha *p) {
    p->topo = 0;
}

bool push(Pilha *p, Peca x) {
    if (pilhaCheia(p)) return false;
    p->dados[p->topo++] = x;
    return true;
}

bool pop(Pilha *p, Peca *out) {
    if (pilhaVazia(p)) return false;
    p->topo--;
    if (out) *out = p->dados[p->topo];
    return true;
}

// -----------------------------
// Geração de peças
// -----------------------------
int g_next_id = 1;

Peca gerarPeca(void) {
    static const char tipos[] = {'I','O','T','L','J','S','Z'};
    int r = rand() % (int)(sizeof(tipos)/sizeof(tipos[0]));
    Peca p = { .tipo = tipos[r], .id = g_next_id++ };
    return p;
}

// -----------------------------
// Exibição
// -----------------------------
void mostrarFila(Fila *f) {
    printf("\n[FILA] (%d/5) frente -> ", f->tamanho);
    if (filaVazia(f)) {
        printf("(vazia)");
    } else {
        for (int i = 0; i < f->tamanho; i++) {
            int idx = idxCircular(f, i);
            Peca p = f->dados[idx];
            if (i == 0) printf("[F]");
            printf(" %c#%d ", p.tipo, p.id);
        }
    }
    printf("\n");
}

void mostrarPilha(Pilha *p) {
    printf("[PILHA/Reserva] (%d/3) topo -> ", p->topo);
    if (pilhaVazia(p)) {
        printf("(vazia)");
    } else {
        for (int i = p->topo - 1; i >= 0; i--) {
            Peca x = p->dados[i];
            if (i == p->topo - 1) printf("[T]");
            printf(" %c#%d ", x.tipo, x.id);
        }
    }
    printf("\n");
}

void mostrarEstado(Fila *f, Pilha *p) {
    mostrarFila(f);
    mostrarPilha(p);
    printf("------------------------------------------------------------\n");
}

// -----------------------------
// Operações de jogo (menu)
// -----------------------------
void opcaoJogarPeca(Fila *fila) {
    Peca jogada;
    if (!dequeue(fila, &jogada)) {
        printf("Não há peça para jogar: fila vazia.\n");
        return;
    }
    printf("Você jogou a peça da frente: %c#%d\n", jogada.tipo, jogada.id);
    // Repor automaticamente para manter 5
    Peca nova = gerarPeca();
    if (!enqueue(fila, nova)) {
        // Não deve acontecer pois tiramos 1 antes
        printf("Aviso: não foi possível repor a fila.\n");
    } else {
        printf("Reposição automática: inserida nova peça %c#%d ao final da fila.\n", nova.tipo, nova.id);
    }
}

void opcaoReservarDaFila(Fila *fila, Pilha *pilha) {
    if (pilhaCheia(pilha)) {
        printf("Reserva cheia (3). Use alguma peça reservada antes.\n");
        return;
    }
    Peca frente;
    if (!dequeue(fila, &frente)) {
        printf("Fila vazia: nada para reservar.\n");
        return;
    }
    push(pilha, frente);
    printf("Peça %c#%d movida da FRENTE da fila para a RESERVA (topo).\n", frente.tipo, frente.id);

    // Repor automaticamente na fila
    Peca nova = gerarPeca();
    enqueue(fila, nova);
    printf("Reposição automática: inserida nova peça %c#%d ao final da fila.\n", nova.tipo, nova.id);
}

void opcaoUsarReservada(Pilha *pilha) {
    Peca usada;
    if (!pop(pilha, &usada)) {
        printf("Reserva vazia: não há peça para usar.\n");
        return;
    }
    printf("Você usou a peça reservada do TOPO: %c#%d\n", usada.tipo, usada.id);
    // Fila permanece com 5; não há alteração na fila aqui.
}

void opcaoTrocarFrenteComTopo(Fila *fila, Pilha *pilha) {
    if (filaVazia(fila)) {
        printf("Não é possível trocar: fila vazia.\n");
        return;
    }
    if (pilhaVazia(pilha)) {
        printf("Não é possível trocar: reserva vazia.\n");
        return;
    }
    int idxFrente = fila->frente;
    int idxTopo   = pilha->topo - 1;

    Peca tmp = fila->dados[idxFrente];
    fila->dados[idxFrente] = pilha->dados[idxTopo];
    pilha->dados[idxTopo]  = tmp;

    printf("Troca realizada entre FRENTE da fila (%c#%d) e TOPO da pilha (%c#%d).\n",
           pilha->dados[idxTopo].tipo, pilha->dados[idxTopo].id,  // após troca
           fila->dados[idxFrente].tipo, fila->dados[idxFrente].id);
}

void opcaoTrocar3Primeiros(Fila *fila, Pilha *pilha) {
    if (fila->tamanho < 3) {
        printf("Fila não possui 3 elementos para trocar.\n");
        return;
    }
    if (pilha->topo != 3) {
        printf("A pilha precisa ter exatamente 3 peças para esta operação.\n");
        return;
    }
    // Troca: frente, frente+1, frente+2  <->  topo, topo-1, topo-2
    for (int i = 0; i < 3; i++) {
        int idxF = idxCircular(fila, i);
        int idxP = pilha->topo - 1 - i;
        Peca tmp = fila->dados[idxF];
        fila->dados[idxF] = pilha->dados[idxP];
        pilha->dados[idxP] = tmp;
    }
    printf("Troca realizada: 3 primeiros da FILA <-> 3 da PILHA (ordem inversa do topo).\n");
}

// -----------------------------
// Inicialização com 5 peças
// -----------------------------
void preencherFilaInicial(Fila *fila) {
    while (!filaCheia(fila)) {
        enqueue(fila, gerarPeca());
    }
}

// -----------------------------
// Menu
// -----------------------------
void mostrarMenu(void) {
    printf("\n=== TETRIS STACK — Menu ===\n");
    printf("1 - Jogar peça (remove da frente e repoe)\n");
    printf("2 - Reservar peça (mover frente da fila -> pilha)\n");
    printf("3 - Usar peça reservada (pop da pilha)\n");
    printf("4 - Trocar frente da fila com topo da pilha\n");
    printf("5 - Trocar 3 primeiros da fila com as 3 da pilha\n");
    printf("0 - Sair\n");
    printf("Escolha: ");
}

int main(void) {
    srand((unsigned)time(NULL));

    Fila fila;
    Pilha pilha;
    inicializarFila(&fila);
    inicializarPilha(&pilha);

    preencherFilaInicial(&fila);

    printf("Bem-vindo ao Desafio Tetris Stack (Nivel MESTRE)!\n");
    mostrarEstado(&fila, &pilha);

    int op;
    for (;;) {
        mostrarMenu();
        if (scanf("%d", &op) != 1) {
            // limpar stdin em caso de entrada inválida
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        printf("------------------------------------------------------------\n");
        switch (op) {
            case 1: opcaoJogarPeca(&fila);           break;
            case 2: opcaoReservarDaFila(&fila, &pilha); break;
            case 3: opcaoUsarReservada(&pilha);      break;
            case 4: opcaoTrocarFrenteComTopo(&fila, &pilha); break;
            case 5: opcaoTrocar3Primeiros(&fila, &pilha);    break;
            case 0:
                printf("Saindo. Valeu por jogar!\n");
                return 0;
            default:
                printf("Opção inexistente.\n");
        }
        mostrarEstado(&fila, &pilha);
    }
}
