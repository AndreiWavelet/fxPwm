# fxPwm

PWM por software para Arduino, que permite modulação por largura de pulso em qualquer porta com total controle de ciclo de trabalho e frequência.

Esse software está liberado em domínio público. Você pode usar livremente para qualquer propósito, mas SEM QUALQUER GARANTIA. Consulte o documento de licença (LICENSE) para mais informação.

## Como Usar

A receita padrão para usar a biblioteca é como se segue:

### 1. Initializar a biblioteca e iniciar o modulador:

fxPwm.Initialize();
fxPwm.Start();

### 2. Registrar pino:

fxPwm.RegisterPort(pinNumber);

Você deve registrar todos pinos que usar. Por padrão, você pode registrar até 32 pinos.
Se precisar de mais pinos, use fxPwm.Initialize(maxPins) na inicialização.

### 3. Configurar a frequência dos pinos.

fxPwm.SetFrequency(pinNumber, frequency);

Você também deve configurar a frequência de cada pino. Escolha a frequência apropriada para sua aplicação. 300 Hz é um bom começo para controle de brilho de um LED.
Frequências baixas tem melhor resolução de ciclo de trabalho.
Frequências altas podem ter jitter. Usar muitas portas em alta frequência pode não ser uma boa ideia.

### 4. Habilitar pinos:

fxPwm.EnablePin(pinNumber);
fxPwm.EnableAll();

### 5. Mudar ciclo de trabalho de qualquer forma que quiser.

fxPwm.SetDuty(pinNumber, dutyCycle);

### 6. Desabilitar PWM, remover portas e liberar.

fxPwm.DisablePin(pinNumber);
fxPwm.DisableAll();
fxPwm.RemovePort(pinNumber);
fxPwm.Stop();
fxPwm.Free();

Na maioria dos casos da vida real, esse passo é desnecessário, já que o microcontrolador é esperado ficar roando por longos períodos.

## Funções de Iniciação de Liberação

### fxPwm.Initialize() fxPwm.Initialize(maxPorts)

Configura a biblioteca com a quantidade máximo padrão de portas, ou define o máximo de portas por maxPorts.

### fxPwm.Free()

Libera quaisquer recursos usados pela biblioteca.

### fxPwm.Start();

Começa a modulação.

### fxPwm.Stop();

Para a modulação.

## Funções de Manipulação de Portas

### fxPwm.RegisterPort(pinNumber);

Registra um pino para ser usado como saída PWM.

### fxPwm.RemovePort(pinNumber);

Remove a porta associada com um pino.

### fxPwm.SetPeriod(pinNumber, period);

Configura o período de um ciclo PWM, em microssegundos. Note que o verdadeiro período pode variar por conta de limitação de resolução do timer.
Também note que períodos muito curtos (<1000 us) (ou muitas portas usadas de uma vez) podem ter jitter.

### fxPwm.SetFrequency(pinNumber, frequency);

Configura a frequência de um ciclo PWM, em hertz (ciclos por segundo). Note que a verdadeira frequência pode variar por conta de limitação de resolução do timer.
Também note que frequências muito altas (>1000 Hz) (ou muitas portas usadas de uma vez) podem ter jitter.

### fxPwm.SetDuty(pinNumber, duty);

Configura o ciclo de trabalho do ciclo PWM. Por padrão, esse valor vai de 0.0 (0% ciclo de trabalho) até 1.0 (100% ciclo de trabalho).
Se fxPwm.SepMap() tiver sido chamado antes de fxPwm.SetDuty, o ciclo de trabalho será mapeado a uma função diferente.

### fxPwm.SepMap(pinNumber, duty1, value1, duty2, value2);

Mapeio o ciclo de trabalho de forma que o intervalo duty1~duty2 se torna value~value2.
Isso é útil se a variável controlada não está no intervalo 0.0 ~ 1.0.

### fxPwm.EnablePin(pinNumber); fxPwm.DisablePin(pinNumber);

Habilita ou desabilita a modulação em um pino especificado, se estiver registrado.

### fxPwm.EnableAll(); fxPwm.DisableAll();

Habilita ou desabilita a modulação em todos pinos.

## Funções de Aquisição de Dados

### UIN8 fxPwm.GetMaxPorts();

Retorna a quantidade máxima de portas registráveis.

### UINT8 fxPwm.GetNumRegisteredPorts();

Retorna a quantidade de portas atualmente registradas.

### TIME_US fxPwm.GetPeriod(pinNumber);

Retorna o período configurado, em microssegundos, da porta especificada. Se não existir, retorna 0.

### FLOAT fxPwm.GetFrequency(pinNumber);

Retorna a frequência configurada, em hertz, da porta especificada. Se não existir, retorna 0.

### FLOAT fxPwm.GetDuty(pinNumber);

Retorna o ciclo de trabalho configurado e mapeado. Se não existir, retorna 0.

### TIME_US fxPwm.Micros();

Retorna o número de microssegundos passados desde que fxPwm.Start() foi chamado pela primeira vez. Para de contar quando fxPwm.Stop() é chamado, e volta a contar toda vez que fxPwm.Start() é chamado.
A resolução pode variar por causa de limitação do timer.

## Funções Avançadas

### RegisterPort(fxPwm_Port *port); RemovePort(fxPwm_Port *port);

Registra ou remove uma porta a partir de um ponteiro para um objeto fxPwm_Port.
Cuidado ao usar.

### fxPwm_Port* GetPort(pin);

Retorna o ponteiro para um objeto fxPwm_Port para manipulação direta.

### UINT8 GetIndex(pin);

Retorna o índice interno de um pino. Não é recomendado usar essa função, já que o índice pode mudar ao se remover portas.
Se o pino não está registrado, retorna 0xFF.

### UINT8 GetRegisteredPortPinNumber(index)

Retorna o número do pino a partir de um índice interno. Se o índice estiver além dos limites, retorna 0xFF.

## Funções fxPwm_Port Functions (avançado)

### fxPwm_Port::SetPinNumber(pinNumber);

Configura o número do pino de uma porta. Cuidado ao usar isso, já que atribuir o mesmo pino para 2 ou mais portas registradas pode quebrar as coisas.

### TIME_US fxPwm_Port::GetPeriod(); FLOAT fxPwm_Port::GetDuty(); FLOAT fxPwm_Port::GetFrequency();

Retorna os valores configurados. O ciclo de trabalho (duty) está mapeado. Período: microssegundos. Frequência: hertz.

### FLOAT fxPwm_Port::GetRawDuty();

Retorna o valor verdadeiro de ciclo de trabalho, sem mapear. 0.0 ~ 1.0 (0% ~ 100%);

### BOOL fxPwm_Port::GetPinState(); fxPwm_Port::SetPinState(BOOL state);

Lê ou atribui o estado do pino (LOW ou HIGH).
Essa função vai forçar o pino a entrar em um estado de SAÍDA (OUTPUT).

### fxPwm_Port::SetPeriodAndDuty(period, duty); fxPwm_Port::SetFrequencyAndDuty(frequency, duty);

Atribui ambos o período (microssegundos) e o ciclo de trabalho mapeado OU ambos a frequência (hertz) e o ciclo de trabalho mapeado.

### fxPwm_Port::SetPeriod(period); fxPwm_Port::SetDuty(duty); fxPwm_Port::SetFrequency(frequency); 

Atribui o período (microssegundos), ciclo de trabalho mapeado ou frequência (hertz).

### fxPwm_Port::SetMap(duty1, value1, duty2, value2);

Veja fxPwm.SepMap(pinNumber, duty1, value1, duty2, value2) acima; funciona do mesmo jeito, mas não tem número de pino.

### fxPwm_Port::Enable(); fxPwm_Port::Disable();

Habilita ou desabilita modulação nessa porta.

## Outras funções

A biblioteca tem outras funções menos úteis. Veja os códigos fonte.

## Problemas conhecidos

### Comunicação serial pode quebrar as coisas

Normalmente, chamar fxPwm.Start() antes e depois das comunicações serial resolve o problema.

### (Teórico) Sobrecarda da contagem de clock

Teoricamente, dado tempo suficiente, a contagem interna de clock vai sobrecarregar e as coisas vão quebrar.
O tempo necessário para isso acontecer depende da resolução do timer. Com uma resolução de 500 ns e F_CPU a 16 MHz, isso vai acontecer depois de 35 minutos e 47 segundos.
Abaixando a resolução (fxPwm_MinTimerResolution em fxPwm.h) vai aumentar esse tempo.
Definindo TIME_IS_64 antes de incluir a biblioteca vai aumentar demais esse limite com alguma penalidade de desempenho. (a 500 ns, 16 MHz, só vai sobrecarregar depois de 292471 anos!)
