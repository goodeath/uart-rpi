# Introdução
O MQTT (euing Telemetry Transport) é um protocolo para comunicação de mensagens entre dispositivos como sensores e computadores móveis. Neste projeto, integraremos o modelo anterior, que se baseava na comunicação UART entre a placa raspberry e o node MCU, com a comunicação MQTT para uma interface de usuário remota.

# Estrutura do projeto
Segue abaixo a estrutura de diretórios do projeto
```
├── dashboard
│   └── chart.js
│   └── index.html
│   └── mqttws31.min.js
├── nodemcu/uart
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
    ├── system.c
    └── uart
        └── uart.c

```
##### cloud-sensors/nodemcu/ - Obtém os valores registrados pelos sensores, realiza a comunicação via UART com a SBC (Raspberry) e MQTT com a interface remota.

##### cloud-sensors/rpi/examples/ - Possui um programa C utilizando as bibliotecas exportadas

##### cloud-sensors/rpi/lib/ - Pasta com os módulos utilizados na solução

##### cloud-sensors/dashboard/ - Interface remota feita em Javascript

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
countdown: counter
example: cexample
uart: cuart
system: csystem

counter: display.o
	gcc -o display display.o

display.o: display.s lib/lcd.o
	as -o display.o display.s

lib/lcd.o: lib/utils.o lib/gpio.o lib/fileio.o
	as -o lib/lcd.o lib/lcd.s

lib/fileio.o: lib/fileio.s
	as -o lib/fileio.o lib/fileio.s

lib/gpio.o: lib/gpio.s
	as -o lib/gpio.o lib/gpio.s

lib/utils.o: lib/utils.s
	as -o lib/utils.o lib/utils.s

uart/uart.o: uart/uart.c
	gcc -c uart/uart.c -lwiringPi

cexample: examples/countdown.c lib/lcd.s
	gcc -o countdown examples/countdown.c lib/lcd.s

cuart: uart/uart.c lib/lcd.s
	gcc -o uartx uart/uart.c lib/lcd.s -lwiringPi

csystem: system.c lib/lcd.s
	gcc -o system system.c lib/lcd.s -lwiringPi
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
Além de contar com os pinos para comunicação UART com a Node MCU, agora utilizam-se 3 botões para a interface local humana. A função destes botões será explicada posteriormente. 

| Pino | GPIO | Descrição |
| - | - | - |
| 8 | 14 | TX |
| 10 | 15 | RX |
| - | 5 | BTN |
| - | 19 | BTN |
| - | 26 | BTN |

# Comandos
Para troca de informações entre os dispositivos, foram definidos comandos. Cada informação é enviada com 1 byte, onde os três bits menos significativos indicam um comando:

| B2 | B1 | B0 | Descrição |
| - | - | - | - |
| 0 | 0 | 1 | Solicita status da NodeMCU |
| 0 | 1 | 0 | Solicita status do sensor | 
| 0 | 1 | 1 | Solicita valor do sensor | 

Os bits mais significativos B7-B3, indicam qual sensor vai ser executado o comando: 0 - 31 (32 sensores).


# Arquitetura
Em relação ao projeto anterior, manteve-se a conexão via UART entre a SBC e a node MCU. O que mudou foi a adição de novos dados sendo passados e o protocolo MQTT estabelecido entre a Node e a interface local, intermediada pelo broker do laboratório.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206821300-396da649-2ab6-47a5-a76c-c27b5aeed07d.png">
</p>

# Funcionamento

## NodeMCU
Os valores dos sensores e seus status foram armazenados em dois vetores de 32 posições. Uma vez que a informação está presente, é recuperada de maneira genérica pela estrutura da informação, onde é separado informação e sensor associado. Desta forma, caso se queira adicionar um novo sensor, basta garantir que a informação vai estar presente na posição escolhida para o mesmo.

A NodeMCU fica constantemente ouvindo o seu canal RX, e toda vez que recebe um pedido, efetua os procedimentos anteriores para retornar a resposta.

## Raspberry PI

Utilizando a implementação para o UART feita no projeto anterior como biblioteca, aqui a nova função para a raspberry é assumir o papel de interface humana local. O usuário, a partir de botões da placa, poderá alterar o sensor, modo de funcionamento e tempo do intervalo. Ao mesmo tempo, todas estas mudanças são registradas no visor LCD.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206802344-a4ec6918-a24b-4853-912d-c05f2932c31e.png">
</p>

Inicialmente, define-se 4 constantes. Cada constante representa um comando que será interpretado pela NodeMCU. **_MODE_SENSOR_** (0) é o modo em que será exibido o valor atual do sensor escolhido e também possibilitará que o usuário possa alternar entre os possíveis sensores. **_MODE_FREQUENCY_** (1) é o modo em que o usuário pode alterar o valor atual do tempo de intervalo da comunicação e esta alteração afetará a interface remota, além de, claro, visualizar no LCD o valor atual vingente. No modo **_MODE_MCU_STATUS_** (2), mostra o status de comunicação da NodeMCU. **_MODE_SENSOR_STATUS_** (3) tal qual o modo anterior, mostra o status de conexão, porém do sensor específico. 

Abaixo, há outras constantes e variáveis úteis para facilitação de entendimento do código. **_TOTAL_SENSOR_** representa o número totais de sensores da comunicação, ou seja 1 analógico e 2 digitais. A variável **_current_screen_** inicializa o programa em modo de tela de valor de sensor. Em **_current_frequency_** a frequência de tempo inicial é definida como 5. 

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206805771-86f8b482-39ec-4830-955f-70b99ba94ebc.png">
</p>

Na imagem acima, ilustra-se 4 funções que definem a lógica por trás da seleção de sensores e alteração da frequência. Numa comunicação UART, só é possível enviar 8 bits de informação. Dito isso, pré-definiu-se que os 5 primeiros bits representarão o número do sensor ou o valor da frequência, e os 3 últimos o a seleção de comandos. Assim, como tem-se 5 bits reservados a sensor e frequência, o usuário pode selecionar do sensor 00000 (0) ao sensor 11111 (31), totalizando 32 sensores. Da mesma maneira, como os mesmos 5 bits são reservados a frequência, o usuário pode alterar a frequência de 0 a 31 segundos. Como dito anteriormente, os 3 últimos bits são responsáveis por decidir o comando. 001 para status do nodemcu, 010 para status do sensor, 011 para o valor do sensor e 100 para frequência. 

Iniciando pela primeira função **_get_mcu_status_code_**, aqui meramente é retornado o valor 1 ou, em binário de 8 bits, XXXXX001, sendo os 5 primeiros bits em _don't care_ já que o número do sensor ou a frequência são irrelevantes para a tela de status da node. Em sequência, **_get_sensor_status_code_** recebe o valor do sensor atual, faz um deslocalamento em 3 bits (multiplica por 8) e soma com 2. Dessa forma, se, por exemplo, o usuário deseja obter o status do sensor 2, a função retornará (2*8)+2, ou seja, 18, que, em binário de 8 bits é representado por 00010 (Sensor 2) 010 (Comando 2). As funções **_get_read_sensor_code_** e **_get_set_frequency_code_** seguem exatamente a mesma lógica anterior, a primeira recebe o sensor novamente, multiplica por 8 e soma com 3 (Teria-se o binário 00010 011 pegando o mesmo sensor do exemplo anterior) e a segunda recebe a frequência, também multiplica por 3 e adiciona ao comando 4, dessa forma, caso o usuário deseje estabelecer a velocidade como 22 segundos, por exemplo, teria-se o valor 22*8 + 4, ou 180, que em binário é representado como 10110 (22) 100 (4).

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206810819-66bda995-6fa9-4ea3-a13e-815ae520b5c5.png">
</p>

Já na função principal, a comunicação UART é inicializada e os 3 botões de interface local humana são pinados. O botão de pino 05 é resposável por alterar a tela atual. Os pinos 19 e 26 são responsáveis por, respectivamente, voltar ou avançar na seleção de sensores ou diminuir e aumentar o valor da frequência. Em sequência cria-se algumas variáveis para ajudar a legibilidade do código. 

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206811117-6140b1ee-2395-4d87-8915-9ada5b623883.png">
</p>

Entrando em loop, o sistema lê o valor atual do pino 05, caso esteja pressionado, a váriavel **_current_screen_** é incrementada. Como só existem 4 telas possíveis, o valor da tela atual retorna ao ponto inicial (0) quando o usuário pressionar o botão 05 pela 4° vez. Após isso, gera-se um delay de 500 milissegundos para evitar o fator _boucing_ do botão. Neste ponto, nota-se que o botão 05 é o principal, pois ele decide em que modo a interface local irá operar, podendo alterar a função dos dois botões de avanço e retardo ou, até mesmo, inutilizá-los em telas que não é necessário alterar sensor ou frequência. 

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206812016-00b6c692-2359-4a74-b387-125c2735f3c9.png">
</p>

Em sequência, verifica-se se o valor da tela atual corresponde ao modo de sensor (0), caso sim, a condição é atendida e é lido os valores dos botões 19 e 26. Caso o botão 19 tenha sido pressionado, o sensor atual (armazenado na variável **_current_sensor_** é decrementado, caso seja o botão 26, o sensor atual é incrementado. A constante **_TOTAL_SENSOR_** é útil para, na operação de divisão e resto, forçar a contagem variar entre 0, 1 e 2 (3 sensores). Feito isso, independente se foi incrementado ou não, obtém-se o comando (**_cmd_**) chamando a função **_get_read_sensor_code_** e passando o sensor atual. Finalmente, o comando obtido é passado pela UART chamando a função **_send_data_**.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206812744-2cabde9a-461e-4418-9f9d-f2bcebba5bd8.png">
</p>

Caso o valor da tela corresponda ao modo de frequência (1), novamente lê-se o valor dos botões 19 e 26. Da mesma forma que 19 volta ao sensor anterior, aqui 19 diminui o valor da frequência atual, enquanto 26 aumenta este valor. As divisões e operações de resto com 0 e 31 servem para manter a contagem alternando de 0 segundos a 31 segundos. Da mesma forma, o valor da frequência é enviado para a função **_get_set_frequency_code_** e obtém-se o comando, o qual é enviado para a node mcu. 

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206813125-5286cea4-1a22-4d43-b4b9-48e116ed93ea.png">
</p>

Uma vez na tela de status da node (2), os botões de avanço e recuo são inúteis, bastando retornar diretamente o valor da função **_get_mcu_status_code_**. O status foi definido de forma fixa como 1.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206813295-501be99c-4865-420d-b613-b0f7aa15328a.png">
</p>

Por fim, na tela de status de sensor (3), repete-se a mesma lógica utilizada no modo de sensor, botão 19 para recuo, 26 para avanço, chama a função específica para esta operação e retorna o comando a node mcu através da UART.

## Interface Remota

A interface remota foi feita em linguagem de marcação HTML e programação em javascript.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206814111-7f3c17d6-88d5-49f6-be32-147a70c22e22.png">
</p>

Inicialmente se define os parâmetros da conexão em MQTT. Os valores para host, porta, usuário e senha foram pré-definidos no _broker_ do laboratório LEDS. Após isso cria-se 2 tópicos, um para atualização dos sensores (lê (subscriber) o valor e os nomes do sensores publicados (publisher) pela Node MCU) e outro para alterar a frequência (publisher). 

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206814911-aacbcf32-d775-4c98-9493-8e0d2f03ea89.png">
</p>

A constante **_onMessageArrived_** é utilizada para obter os dados dos sensores da Node MCU. Inicialmente, obtém-se o nome do tópico e status da conexão. Caso o tópico corresponda ao tópico de atualização e a conexão esteja habilitada, a condição é atendida. Em seguida, obtém o valor da data atual, exclui-se a primeira data registrada na conexão (a mais antiga) e atualiza o histórico para as 10 mais novas (com a atual). A informação da conexão é parseada para o formato _json_, de modo a utilizar a função map e obter um array de objetos contendo o atributo label (nome do sensor) e data (valor do sensor). Por fim, limpa-se o gráfico em **_removeCharts_** e adiciona os novos dados em **_addData_**. Tais funções serão explicadas posteriormente.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206818257-6cb289e1-b370-4f50-8b2c-1704314717b6.png">
</p>

A constante **_onConnect_** é útil para estabelecer a interface remota como _subscriber_ do tópico de atualização. Utiliza-se a biblioteca PAHO para utilizar as funções do protocolo MQTT. Aqui chama-se a função **_mqtt.subscribe_** para inscrever-se no tópico de atualização e lança uma mensagem caso haja conexão.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206818708-bde18080-1e4c-4ba2-9d89-ea37ba6401d3.png">
</p>

Por fim, estabelece-se a conexão MQTT passando os parâmetros de host e porta definidos anteriormente. Em caso de uma mensagem recebida, chama-se a constante **_onMessageArrived_** para obter os nomes e valores de sensores explicados anteriormente. Por fim, a conexão é efetivamente iniciada em **_mqtt.connect_**, passando os parâmetros de usuários e senha. Caso haja conexão, a constante **_onConnect_** é chamada para inscrever a interface ao tópico.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206819063-cb68d9d4-5f1d-49b0-856b-f19507d1e514.png">
</p>

Neste ponto, cria-se na interface uma caixa de input. Nela, o usuário digita o valor da frequência que será enviado ao node mcu. Em seguida, e logo abaixo na interface, cria-se um canvas, corpo de HTML que permite edição de javascript. Neste canvas é implementado o gráfico do histórico de valor dos sensores.

Na área de script, adiciona um evento de **_listening_** para o formulário de frequência. A cada vez que o usuário atualiza o valor vingente, o programa resgata tal valor e o publica no tópico de frequência utilizando a função **_mqtt.send**.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206819505-594df292-9c49-4b2c-8aca-9418782fc478.png">
</p>

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206819543-5de759bc-5ad4-4fbe-8b49-e44e48b40a4a.png">
</p>

Em sequência, cria-se o gráfico utilizando a biblioteca Chart na área do canvas. Inicialmente, o gráfico é preenchido com valores aleatórios, apenas para ocupar a tela enquanto os valores da conexão não são recebidos.

<p align="center">
	<img src="https://user-images.githubusercontent.com/88406625/206819657-58c73d9f-6fe2-4260-8648-5c5b004f5b98.png">
</p>

Por fim, têm-se as duas funções citadas anteriormente. A função **_addData_** substitui os _labels_ do gráfico pelo vetor que armazena os nomes de todos os sensores conectados a node mcu e atualiza os dados pelos respectivos valores temporais de cada sensor. Já a função **_removeData_** apenas esvazia os registros do gráfico, esvaziando os vetores.

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
![ezgif com-gif-maker](https://user-images.githubusercontent.com/26310730/200437854-fd1294f9-dee1-4beb-9a84-4857ef3f05ec.gif)

O protótipo construído é um sistema digital utilizando plataformas de desenvolvimento IoT, em que se pode adicionar até 32 sensores analógicos e/ou digitais e exibir as respectivas informações em um display de LCD.

## Limitações da UART

### Quantidade de dispositivos
A comunicação serial, apesar de simples, mas só permite a comunicação entre dois dispositivos. Caso seja necessário enviar ou receber informações de mais dispositivo se torna inviável, necessitando do uso de outros protocolos, como o caso do i2c;

### Velocidade
Comparado a protocolos como i2c e SPI, pode haver uma diferença de velocidade de até 10 vezes, fazendo o protocolo UART ser mais lento e algumas vezes inviável dependendo da aplicação.
