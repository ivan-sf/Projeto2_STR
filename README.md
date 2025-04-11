# Projeto STR 2024.2 — Simulação de Célula de Manufatura com FreeRTOS

## Descrição

Este projeto implementa a simulação de uma célula de manufatura utilizando o sistema operacional de tempo real **FreeRTOS**, com o objetivo de modelar o funcionamento de robôs e máquinas em um ambiente de produção industrial.
## Vídeo de Demonstração
Assista à demonstração e explicação do projeto no YouTube:  
https://youtu.be/gutfN_xmWy4

## Funcionalidades

- Simulação com FreeRTOS utilizando tarefas (tasks) para robôs e máquinas.
- Sincronização com semáforos e filas para controlar acesso aos depósitos.
- Modelagem precisa de tempos de produção, transporte e armazenamento.
- Controle de produção com movimentação de itens do início ao fim da célula.
- Detecção de paradas de máquinas e gargalos de produção.
- Execução no ambiente Windows usando o projeto WIN32 do FreeRTOS.
- Saída de mensagens no terminal com status do sistema em tempo real.

## Regras da Célula de manufatura

- **Depósitos**: entrada/saída da célula e depósitos individuais para cada máquina.
- **Máquinas**:
  - M1: produz a cada 1.5 s
  - M2: produz a cada 1.5 s
  - M3: produz a cada 3 s
- **Robôs**:
  - R1: Entrada da célula → Entrada de M1
  - R2: Saída de M1 → Entrada de M2
  - R3: Saída de M1 → Entrada de M3
  - R4: Saída de M2 e M3 → Saída da célula
 
## Restrições e Tempos

- Robôs levam:
  - 0.1 s para colocar/retirar itens
  - 0.5 s para se mover (exceto R3: 0.8 s)
- Apenas **um robô por vez** pode acessar o depósito de M1.
- Depósitos armazenam **apenas um item** por vez.
- Máquinas param se:
  - O depósito de saída estiver cheio
  - O depósito de entrada estiver vazio
## Exemplo de Execução

```plaintext
(1 ms) [R1] Pegou o item #1 do deposito de entrada da celula.
(701 ms) [R1] Colocou o item #1 no deposito da M1.
(701 ms) [M1] Comecou a processar o item #1.
(2201 ms) [M1] Finalizou o processamento do item #1.
(2201 ms) [M1] Colocou o item #1 processado no deposito de saida.
(2201 ms) [R2] Pegou o item #1 processado de M1.
(2201 ms) [R1] Pegou o item #2 do deposito de entrada da celula.
(2901 ms) [R2] Colocou o item #1 no deposito de M2.
(2902 ms) [R1] Colocou o item #2 no deposito da M1.
(2902 ms) [M2] Comecou a processar o item #1.
(2902 ms) [M1] Comecou a processar o item #2.
(4402 ms) [M2] Finalizou o processamento do item #1.
(4402 ms) [M2] Colocou o item #1 processado no deposito de saida.
(4402 ms) [M1] Finalizou o processamento do item #2.
(4402 ms) [M1] Colocou o item #2 processado no deposito de saida.
(4402 ms) [R1] Pegou o item #3 do deposito de entrada da celula.
(4402 ms) [R3] Pegou o item #2 processado de M1.
(4452 ms) [R4] Pegou o item #1 da M2.
(5102 ms) [R1] Colocou o item #3 no deposito da M1.
(5102 ms) [M1] Comecou a processar o item #3.
(5152 ms) [R4] Colocou o item #1 na saida da celula.
(5402 ms) [R3] Colocou o item #2 no deposito de M3.
(5403 ms) [M3] Comecou a processar o item #2.
(6602 ms) [M1] Finalizou o processamento do item #3.
(6602 ms) [M1] Colocou o item #3 processado no deposito de saida.
(6602 ms) [R1] Pegou o item #4 do deposito de entrada da celula.
(6602 ms) [R2] Pegou o item #3 processado de M1.
(7302 ms) [R1] Colocou o item #4 no deposito da M1.
(7302 ms) [R2] Colocou o item #3 no deposito de M2.
(7303 ms) [M1] Comecou a processar o item #4.
(7303 ms) [M2] Comecou a processar o item #3.
...
```

## Como Executar
Pré-requisitos:
- **Visual Studio** (Versão Community 2022 utiliada)
- SDK do windows
- Pacote de desenvolvimento c++ (Há opção de instalação juntamente com o Virtual studio)
- FreeRTOS v10.0.1 (já incluído no projeto)

Compilação e Execução
# Clone o repositório
```plaintext
git clone https://github.com/ivan-sf/Projeto2_STR/
```
# Abra o código no Visual Studio
Seguindo os seguintes passos:
- Canto superior esquerdo: Abrir
- Selecione: Projeto/Solução
- Abra a pasta do projeto
- Selecione o arquivo WIN32.sln
O virtual studio carregará o programa.
Caso não tenha o pacote de desenvolvimento c++ será necessário instalar;
# Compilar o programa
Para compilar o programa:
- aba de ferramentas superior: Compilação
- subitem: Compilar solução
Alternativamente, existe o atalho `ctrl+Shift+B`
# Executar o programa
Na aba de ferramentas superior, Depuração:
- Depurar aplicação, atalho: `F5`
ou
- Iniciar sem depurar, atalho: `Ctrl + F5`
- subitem: Compilar solução
 ## Análise de Sincronização
 Caso ocorra erros na compilação sobre o SDK do windows, siga os seguintes passos:
 - Aba de gerenciador de soluções(Atalho `Ctrl+ç`)
 - Clique direito com o mouse em Solução 'WIN32'
 - Redirecionar solução
 - Altere a versão do SDK

## Contribuições
Contribuições são bem-vindas! Siga os passos:

Faça um fork do projeto.

Crie uma branch: git checkout -b feature/nova-funcionalidade.

Commit suas mudanças: git commit -m 'Adicione uma funcionalidade'.

Push para a branch: git push origin feature/nova-funcionalidade.

Abra um Pull Request.



## Autores
Ádson Vital Correia (adson.correia@ee.ufcg.edu.br); 

Arthur de Queiroz Tavares Borges Mesquisa (arthur.mesquita@ee.ufcg.edu.br)

Ivan da Silva Filho (ivan.filho@ee.ufcg.edu.br)

## Licença
Distribuído sob a licença MIT. Veja LICENSE para detalhes.
