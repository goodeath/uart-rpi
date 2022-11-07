# Introdução
UART-RPI é um projeto de comunicação serial utilizando a Raspberry Pi e o NodeMCU através da UART. Apesar de ser uma comunicação de curto alcance e em geral lenta, ecom esta integração, é possível recuperar informações de sensores instalados no NodeMCU e transmiti-los para a raspberry. É importante por alguns motivos, entre eles, a aquisição de dados dos sensores é mais simples ser realizada na NodeMCU e também, na disponibilidade de outro sistema de aquisição, neste caso, mais barato, conseguimos economizar GPIOs do nosso sistema central. 


# Estrutura do projeto
Segue abaixo a estrutura de diretórios do projeto
```
├── nodemcu
│   └── uart.ino
├── README.md
└── rpi
    ├── display.s
    ├── examples
    │   └── countdown.c
    ├── lib
    │   ├── fileio.s
    │   ├── gpio.s
    │   ├── lcd.s
    │   └── utils.s
    ├── LICENSE
    ├── makefile
    └── uart
        └── uart.c

```
##### nodemcu/ - Possui um programa C que executa na nodemcu para tratamento da comunicação/sensores

##### rpi/examples/ - Possui um programa C utilizando as bibliotecas exportadas

##### rpi/lib/ - Pasta com os módulos utilizados na solução

## Bibliotecas
#### lib/fileio.s
Possui a macro open_file para abertura de arquivos. Recebe no R0, o descritor do arquivo aberto, no R1, o modo de abertura do arquivo.

#### lib/utils.s
Possui a macro nanosleep para fazer o programa parar durante o tempo específicado. R0 é um ponteiro para quantidade de segundos e R1 é um ponteiro para quantidade de nanossegundos.
#### lib/gpio.s
Possui macros para configurar pinos como entrada e saída, alterar o nível lógico no modo de saída e ler o nível lógico em determinado pino. A sessão de pinos tem seu array configurado da seguinte maneira:
#### lib/lcd.s
Biblioteca principal para o controle do LCD
#### display.s

Programa principal para execução do contador. O valor do contador fica registrado em R1, e as flags para pausar/continuar e reiniciar contagem, estão nos registradores R6 e R5, respectivamente

# Makefile

Para facilitar a construção do programa, existe um makefile dentro da pasta rpi, onde é possível executar:
`$ make uart`
Para construção do executável. Logo em seguida basta utilizar:
`$ sudo ./uartx`
para executar o programa
```
uart: cuart
cuart: uart/uart.c lib/lcd.s
	gcc -o uartx uart/uart.c lib/lcd.s -lwiringPi
```
# Dispositivos

Abaixo está presente os dispositivos utilizados, suas características e documentação utilizada para desenvolvimento do projeto

# NodeMCU
A plataforma NodeMCU é uma placa de desenvolvimento que combina o chip ESP8266, uma interface usb-serial e um regulador de tensão 3.3V.  Mais dados sobre sua documentação podem ser encontrados [aqui](https://nodemcu.readthedocs.io/en/release/).

Alguns pinos utilizados na NodeMCU estão listados na tabela abaixo:

| Pino | Descrição |
| - |  - |
| D0 | Sensor Digital 1 |
| D1 | Sensor Digital 2 |
| A0 | Sensor Analógico 1 |
| TX | Envio comunicação serial |
| RX | Recebimento comunicação serial |


## Raspberry Pi Zero
Baseada no processador [BCM 2385](https://datasheets.raspberrypi.com/bcm2835/bcm2835-peripherals.pdf), possui 54 I/O de propósito geral (GPIO), além daqueles utilizados para comunicação com o display, estão sendo utilizados mais dois para comunicação serial: TX/RX. É importante notar que, o GPIO 1 não está posicionado no PINO 1. As informações da placa são mostradas na tabela abaixo, junto da descrição sobre o uso de cada GPIO.

| Pino | GPIO | Descrição |
| - | - | - |
| 8 | 14 | TX |
| 10 | 15 | RX |


### Rotina de Inicialização

Para realizar a inicialização completa do display, é preciso executar os seguintes passos:

| E | DB7 | DB6 | DB5 | D4 |
| - | - | - | - | - |
|  ⎍  | 0 | 0 | 1 | 1 |
|  ⎍  | 0 | 0 | 1 | 1 |
|  ⎍  | 0 | 0 | 1 | 1 |
|  ⎍  | 0 | 0 | 1 | 0 |
|  ⎍  | 0 | 0 | 1 | 0 |
|  ⎍  | 1 | 0 | 0 | 0 |
|  ⎍  | 0 | 0 | 0 | 0 |
|  ⎍  | 1 | 0 | 0 | 0 |
|  ⎍  | wait | 5 | m | s |
|  ⎍  | 0 | 0 | 0 | 0 |
|  ⎍  | 0 | 0 | 0 | 1 |
|  ⎍  | wait | 5 | m | s |
|  ⎍  | 0 | 0 | 0 | 0 |
|  ⎍  | 0 | 1 | 1 | 0 |
|  ⎍  | wait | 5 | m | s |
|  ⎍  | 0 | 0 | 0 | 0 |
|  ⎍  | 1 | 1 | 1 | 1 |

As transições são reconhecidas nas bordas de descida.

# Comandos
Para troca de informações entre os dispositivos, foram definidos comandos. Cada informação é enviada com 1 byte, onde os três bits mais significativos indicam um comando:

| B2 | B1 | B0 | Descrição |
| - | - | - | - |
| 0 | 0 | 1 | Solicita status da NodeMCU |
| 0 | 1 | 0 | Solicita status do sensor | 
| 0 | 1 | 1 | Solicita valor do sensor | 

Os bits mais significativos B7-B3, indicam qual sensor vai ser executado o comando: 0 - 31 (32 sensores).


# Arquitetura
A arquitetura utilizada é a ArmV6 presente na Raspberry Pi Zero, pertencente a família de processadores RISC. A Raspberry Pi Zero utiliza o processador BCM 2835, que é um processador de 32 bits e seu programa pode ser executado em versões mais recentes, pois o ArmV7 e ArmV8 possui retrocompatibilidade com o ArmV6.

Os registradores da arquitetura Arm é composta por:
- 13 registradores (R0-R12) de uso geral;
- 1 Stack Pointer (SP);
- 1 Link Register (LR);
- 1 Program Counter (PC);
- 1 Application Program Status Register APSR

Se tratando do sistema operacional, registradores podem ter usos específicos, cuidado! Por exemplo, no Raspberry Pi OS, a função de sistema é escolhida colocando seu identificador no R7, e seu retorno está presente no R0 após a chamada da função.


# Sistema Operacional
O sistema operacional utilizado é o Raspberry Pi OS, derivado do Debian. Desta maneira, possui as mesmas chamadas de sistema que o linux.
## System Calls

Devido a presença de sistema operacional na Raspberry Pi Zero, para realizar um acesso aos dispositivos presentes na placa, é necessário realizar o mapeamento de memória, onde é exigido a chamada de algumas system calls para realizar esse acesso. Para outros dispositivos, como o acionamento do relógio interno, o sistema operacional oferece uma interface amigável.

##### void _exit(int status);
Finaliza o programa com um código de status. Caso o status seja 0, o programa foi encerrado com sucesso. Seu código é 1.
```
.EQU sys_exit, #0x1
.global _start
_start:
    LDR R0, =0x0
    LDR R7, =#sys_exit
    SVC 0
.data
```

##### int open(const char *pathname, int flags);
Abre um arquivo e retorna seu descritor. O primeiro parâmetro é o nome do arquivo e o segundo indica a maneira que o arquivo vai ser aberto. Esta chamada é feita para abertura do arquivo /dev/mem onde é o responsável por receber entradas para realização do mapeamento de memória.  Seu número de chamada é 5.

```
.EQU sys_open, #0x5
.global _start
_start:
    LDR R0, =file
    LDR R1, =mode
    LDR R1, [R1]
    LDR R7, =#sys_open
    SVC 0
.data
mode: .word 2
```

##### int nanosleep(const struct timespec *req, struct timespec *rem);
Utilizada para fazer o programa aguardar (\*req) segundos + (\*rem) nanossegundos. Dentro do projeto do contador, esta chamada auxilia o processo de contagem dos segundos. Seu número de chamada é 162. Segue um exemplo de uso, fazendo com o que o código espere 5,5 segundos:
```
.EQU nanosleep_sys, #162
.global _start
_start:
    LDR R0, =time_in_seconds
    LDR R1, =time_in_nano
    LDR R7, =#nanosleep_sys
    SVC 0
.data
time_in_seconds: .word 5
time_in_nano: .word 500000000
```
 
##### void *syscall(SYS_mmap2, unsigned long addr, unsigned long length, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long pgoffset);
Utilizada para mapear um endereço físico para memória virtual, retornando seu endereço para uso do programa.
```
.EQU mmap2_sys, #192
.EQU pagelen, 4096
.equ MAP_SHARED, 0x01
.equ FLAG, 0x7
.global _start
    LDR R0, =0
    LDR R1, =#pagelen
    LDR R2, =#FLAG
    LDR R3, =#MAP_SHARED
    LDR R5, =gpio_base_addr
    LDR R5, [R5]
    LDR R7, =#mmap2_sys
    SVC 0
.data
gpio_base_addr: .word 0x20200
```


# Interface 
API da biblioteca para manipulação do LCD
##### void init();
Inicializa o display para que possa ser escrito

##### void clear_display();
Faz com que o display inteiro seja limpo

##### void write_char(char ch);
Escreve caracteres alfa numéricos (0-9a-zA-Z)

##### void move_cursor(int x, int y);
Posiciona o cursor na linha x (0-1) e na coluna y (0-15)

# Como executar

### UART - Raspberry Pi
1. Na pasta rpi/ execute:

`$ make uart`

2. Em seguida execute o programa

`$ sudo ./uartx`

### UART - NodeMCU
1. Na pasta nodemcu/ abra o arquivo uart.ino na Arduino IDE:
2. Configure as bibliotecas do NodeMCU
3. Descarregue o código na pltaforma


# Resultados

![contagem ocorrendo](result.gif)

O protótipo construído é um contador de 2 digitos, onde é possível configurar seu valor diretamente no código, possibilitando diferentes tempos. Ainda, é possível utilizar dois botões de controle. Um botão para pausar/iniciar a contagem, onde sua função alterna a cada pressionamento. E um botão para reiniciar a contagem, onde é acionado apenas uma vez após o pressionamento, necessitando assim soltar o botão antes de realizar um novo reinício.

## Limitações da UART

### Quantidade de dispositivos
A comunicação serial, apesar de simples, mas só permite a comunicação entre dois dispositivos. Caso seja necessário enviar ou receber informações de mais dispositivo se torna inviável, necessitando do uso de outros protocolos, como o caso do i2c;

### Velocidade
Comparado a protocolos como i2c e SPI, pode haver uma diferença de velocidade de até 10 vezes, fazendo o protocolo UART ser mais lento e algumas vezes inviável dependendo da aplicação.