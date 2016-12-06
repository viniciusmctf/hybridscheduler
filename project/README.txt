- Estrutura de pastas:
	* scheduler_project: "possui os arquivos do projeto do componente, feito para gerar o componente que sera instanciado no Qsys"
		* qsys_component: "possui toda a arquitetura criada, ja com o bloco Scheduler definido e instanciado"
			* software: "pasta com os projetos de software gerados para a arquitetura produzida"

	* to_fpga "pasta com o projeto que deve ser compilado para a placa"
		*synthesis "Pasta onde ficam os arquivos da arquitetura, gerados pelo Qsys "

	* imagens "pasta com imagens de execucoes dos softwares disponibilizados"
	
ATENCAO: A pasta principal deve estar localizada num caminho sem caracteres especiais!!!

- Softwares:
0 - playground
	Uma sequencia de testes quaisquer, com proposito de meramente demonstrar que eh possivel controlar o hardware

1 - unit_test
	Conjunto de testes de unidade, que pode ser executado para verificar o funcionamento de cada comando.
	ATENCAO: 
		- os testes relacionados a interrupcoes podem demorar para testar, visto que a arquitetura
	possui uma quantidade de Ticks default, que requer um tempo de espera. 
		- os teste relacionados a interrupcoes devem ser executados de forma mutuamente exclusiva.

2 - scheduler_sim 
	Implementacao dos metodos que controlam o Scheduler, simulando enderecos de objetos e prioridade com parametros. 

3 - [EM CONSTRUCAO] scheduler 
	Implementacao do Scheduler.

O que fazer?
1 - Abrir projeto "to_fpga" e compilar.
2 - Programar a placa com o projeto.
	- não fechar a janela "OpenCore Plus Status", com Time remaining, durante o processo
3 - Abrir Eclipse 
	- Tools > Nios II Software Build Tools for Eclipse
4 - Importar projeto e BSP
	- para cada um, faca:
		- File > import > Nios II S. B. T. Project > Import Nios II S. B. T. Project
		- Selecione a pasta do software desejado (ver estrutura de pastas)
		- Recomendamos que seja mantido o nome da pasta selecionada.
		- Next, Finish
5 - Compile
	- botao direito no projeto e Build Project

6 - Execute
	- botao direito no projeto, Run As > Run Configurations
	- Nios II Hardware
		- Na aba project, selecione o projeto desejado
		- Em target connection, selecione a placa na qual o software vai rodar
			- Se a placa estiver conectada e nao aparecer, tente fazer
				marque: Ignore mismatched system ID
				marque: Ignore mismatched system timestamp
				clique em Refresh connections 
	- Clique em Apply, Run.

* Se houver modificacao no componente Scheduler ou Arquitetura em Qsys:
	- Gerar, pelo Qsys, os arquivos no formato verilog para a pasta alvo "to_fpga"
	- é necessário atualizar o BSP no Eclipse
		clicar com botao direito no projeto "bsp > Nios II > Generate BSP"

* A descricao original do "Scheduler.vhd" e seu controlador em C, adaptado para o EPOS, 
	foram adaptados da dissertacao de Marcondes, disponivel em:
	"http://www.lisha.ufsc.br/pub/Marcondes_MSC_2009.pdf"

* O Arquivo "Documentacao.pdf" possui uma descricao de cada comando da arquitetura, com seus parametros, funcionamento
 	e retorno detalhados. Ele tambem possui uma Figura ilustrando a organizacao do bloco desenvolvido.

* O arquivo "Tutorial de interrupcoes.pdf" possui uma descricao de como tratar, em C, as interrupcoes da Avalon.
	O material foi usado de apoio, e encontra-se disponivel aqui para fins de consultas futuras.
	Os creditos pela criacao do material sao de seus autores.