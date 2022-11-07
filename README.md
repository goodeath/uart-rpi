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

Devido a alguns lixos gerados na saída serial da NodeMCU foi realizado um processo de inicialização. Quando a raspberry pi inicia, fica aguardando o envio de um conjunto de palavras em sequência específica para identificar que a inicialização foi feita com sucesso. 

# Comandos
Para troca de informações entre os dispositivos, foram definidos comandos. Cada informação é enviada com 1 byte, onde os três bits mais significativos indicam um comando:

| B2 | B1 | B0 | Descrição |
| - | - | - | - |
| 0 | 0 | 1 | Solicita status da NodeMCU |
| 0 | 1 | 0 | Solicita status do sensor | 
| 0 | 1 | 1 | Solicita valor do sensor | 

Os bits mais significativos B7-B3, indicam qual sensor vai ser executado o comando: 0 - 31 (32 sensores).


# Arquitetura
Como mostrado na figura, temos a SBC controlando a exibição de informações no display, enquanto se comunica através da uart com a NodeMCU que possui e faz a aquisição dos dados dos sensores.

![image](https://user-images.githubusercontent.com/26310730/200289359-d2724ca6-85cb-48ff-bf14-99044af3eb83.png)


# Funcionamento

## NodeMCU
Os valores dos sensores e seus status foram armazenados em dois vetores de 32 posições. Uma vez que a informação está presente, é recuperada de maneira genérica pela estrutura da informação, onde é separado informação e sensor associado. Desta forma, caso se queira adicionar um novo sensor, basta garantir que a informação vai estar presente na posição escolhida para o mesmo.

A NodeMCU fica constantemente ouvindo o seu canal RX, e toda vez que recebe um pedido, efetua os procedimentos anteriores para retornar a resposta.

## Raspberry PI

Pode-se emitir os comandos através do terminal, onde são enviados e processados pela NodeMCU através de comunicação serial utilizando o protocolo UART. Como o processo é assíncrono, é realizada uma espera ocupada de até 1 segundo (aproximadamente), de forma que se não houver nenhum tipo de resposta, é dado como um erro de tempo excedido (timeout).  vez que a informação retorne, ela é exibida no terminal e no display de LCD caso esteja conectado.
Devido a presença de sistema operacional na Raspberry Pi Zero, para realizar um acesso aos dispositivos presentes na placa, é necessário realizar o mapeamento de memória, onde é exigido a chamada de algumas system calls para realizar esse acesso. Para outros dispositivos, como o acionamento do relógio interno, o sistema operacional oferece uma interface amigável.

Para estabelecer a comunicação UART, utilizaram-se as bibliotecas _wiringPi_ e _wiringSerial_ dedicadas a mapeamento de GPIOs em hardwares Raspberry. A taxa de transmissão foi definida como 9600. A imagem abaixo ilustra a função responsável por mapear e retornar o valor da porta serial que representa a mini UART.

![image](https://user-images.githubusercontent.com/88406625/200326392-5edb8280-7448-45ce-840e-659629bdf7fe.png)




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

O protótipo construído é um sistema digital utilizando plataformas de desenvolvimento IoT, em que se pode adicionar até 32 sensores analógicos e/ou digitais e exibir as respectivas informações em um display de LCD.

## Limitações da UART

### Quantidade de dispositivos
A comunicação serial, apesar de simples, mas só permite a comunicação entre dois dispositivos. Caso seja necessário enviar ou receber informações de mais dispositivo se torna inviável, necessitando do uso de outros protocolos, como o caso do i2c;

### Velocidade
Comparado a protocolos como i2c e SPI, pode haver uma diferença de velocidade de até 10 vezes, fazendo o protocolo UART ser mais lento e algumas vezes inviável dependendo da aplicação.
