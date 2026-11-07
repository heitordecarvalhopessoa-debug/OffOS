# OffOS-0.1TEST

Um pequeno sistema operacional experimental para x86, feito em C e Assembly.
Ele inicia por uma ISO com GRUB e mostra uma área de trabalho gráfica inspirada
na imagem de referência.

## O que funciona

- Área de trabalho azul, barra superior e janela inicial **APPS**.
- Mouse PS/2 com cursor desenhado como **⬉**; teclado PS/2 e teclas de atalho.
- **File** abre uma lista de arquivos de demonstração. Os arquivos e seus textos
  são virtuais e fazem parte do kernel; não há disco gravável nesta versão.
- **Tools** abre Calculator, Calendar, Clock e ArtApp.
- Calculator aceita inteiros e as operações `+`, `-`, `*` e `/`; use os botões,
  o teclado numérico, Enter, Backspace e C.
- Calendar mostra o mês atual e permite navegar entre meses.
- Clock lê hora e data do relógio CMOS/RTC da máquina.
- ArtApp permite desenhar na tela com o mouse, escolher cores e limpar o quadro.
- **Quit** pede confirmação e envia o comando de desligamento ACPI usado pelo
  QEMU/Bochs. Se a máquina não aceitar esse comando, o kernel para em segurança.

## Compilar e iniciar

No ambiente de desenvolvimento, instale `gcc`, `nasm`, `grub2`, `xorriso` e
`qemu`; depois, na pasta `offos`:

```sh
make
make run
```

## ATALHOS

- `F`: menu File
- `T`: menu Tools
- `Q`: tela de desligamento
- `ESC`: fecha o menu ou a janela atual

- No Calculator: dígitos, + - * /, Enter, Backspace e C
- No ArtApp: clique e arraste para desenhar; C limpa o quadro