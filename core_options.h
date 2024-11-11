/*
 *  Copyright (C) 2020-2023 Bernhard Schelling
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

static retro_core_option_v2_category option_cats[] =
{
	{ "Emulation",   "Emulação",   "Configurações específicas do núcleo (latência, estados de salvamento, menu de início)." },
	{ "Input",       "Entrada",       "Configurações de teclado, mouse e joystick." },
	{ "Performance", "Desempenho", "Ajuste o desempenho da CPU emulada." },
	{ "Video",       "Vídeo",       "Configurações para a placa gráfica emulada e proporção de aspecto." },
	{ "System",      "Sistema",      "Outras configurações do sistema para a RAM e CPU emuladas." },
	{ "Audio",       "Áudio",       "Configurações de MIDI, SoundBlaster e outras configurações de áudio." },
	{ NULL, NULL, NULL }
};

static retro_core_option_v2_definition option_defs[] =
{
	{
		"dosbox_pure_advanced",
		"Mostrar Opções Avançadas", NULL,
		"Feche e reabra o menu para atualizar esta página de opções.", NULL,
		NULL,
		{ { "false", "Desativado" }, { "true", "Ativado" } },
		"false"
	},
	// Emulation
	{
		"dosbox_pure_force60fps",
		"Forçar saída de 60 FPS", NULL,
		"Ative essa opção para forçar a saída a 60FPS. Use essa opção se tiver problemas de tremulação de tela ou de sincronização de vídeo.", NULL,
		"Emulação",
		{
			{ "false", "Ativado" },
			{ "true", "Desativado" },
		},
		"false"
	},
	{
		"dosbox_pure_perfstats",
		"Mostrar estatísticas de desempenho", NULL,
		"Ative essa opção para mostrar estatísticas sobre desempenho e taxa de quadros e verificar se a emulação é executada em velocidade máxima.", NULL,
		"Emulação",
		{
			{ "none",     "Desativada" },
			{ "simple",   "Simples" },
			{ "detailed", "Informações detalhadas" },
		},
		"none"
	},
	{
		"dosbox_pure_savestate",
		"Suporte para salvar estados", NULL,
		"Certifique-se de testá-lo em cada jogo antes de usá-lo. Jogos complexos do DOS do final da era podem ter problemas." "\n"
		"Lembre-se de que os estados salvos com configurações diferentes de vídeo, CPU ou memória não podem ser carregados." "\n"
		"O suporte ao rebobinamento tem um alto custo de desempenho e precisa de pelo menos 40 MB de buffer de rebobinamento.", NULL,
		"Emulação",
		{
			{ "on",       "Ativar estados de salvamento" },
			{ "rewind",   "Ativar estados de salvamento com rebobinagem" },
			{ "disabled", "Desativado" },
		},
		"on"
	},
	{
		"dosbox_pure_strict_mode",
		"Avançado > Usar Modo Estrito", NULL,
		"Desabilita a linha de comando, executando sistemas operacionais instalados e utilizando arquivos .BAT/.COM/.EXE/DOS.YML a partir do jogo salvo.", NULL,
		"Emulação",
		{
			{ "false", "Ativado" },
			{ "true", "Desativado" },
		},
		"false"
	},
	{
		"dosbox_pure_conf",
		"Avançado > Carregamento de dosbox.conf", NULL,
		"O DOSBox Pure deve ser configurado via opções principais, mas opcionalmente suporta o carregamento de arquivos .conf legados.", NULL,
		"Emulação",
		{
			{ "false", "Suporte a conf desabilitado" },
			{ "inside", "Tentar 'dosbox.conf' no conteúdo carregado (ZIP ou pasta)" },
			{ "outside", "Tentar '.conf' com o mesmo nome do conteúdo carregado, ao lado do ZIP ou pasta. (padrão)" },
		},
		"outside"
	},
	{
		"dosbox_pure_menu_time",
		"Avançado > Menu Iniciar", NULL,
		"Definir o comportamento do menu Iniciar antes e depois de iniciar um jogo." "\n"
		"Você também pode forçar a abertura mantendo pressionada a tecla Shift ou L2/R2 ao selecionar 'Reiniciar'.", NULL,
		"Emulação",
		{
			{ "99", "Mostrar no início, mostrar novamente após a saída do jogo (padrão)" },
#ifndef STATIC_LINKING
			{ "5", "Mostrar no início, desligar o núcleo 5 segundos após a saída do jogo iniciado automaticamente" },
			{ "3", "Mostrar no início, desligar o núcleo 3 segundos após a saída do jogo iniciado automaticamente" },
			{ "0", "Mostrar no início, desligar o núcleo imediatamente após a saída do jogo iniciado automaticamente" },
#endif
			{ "-1", "Sempre mostrar menu na inicialização e após a saída do jogo, ignorar a configuração de início automático" },
		},
		"99"
	},
	{
		"dosbox_pure_latency",
		"Avançado > Latência de Entrada", NULL,
		"Por padrão, o núcleo opera em modo de alto desempenho com boa latência de entrada." "\n"
		"Há um modo especial disponível que minimiza ainda mais a latência de entrada, exigindo ajustes manuais.", NULL,
		"Emulação",
		{
			{ "default", "Padrão" },
			{ "low", "Latência mais baixa - Consulte a configuração de uso da CPU abaixo!" },
			{ "variable", "Latência irregular - Pode melhorar o desempenho em dispositivos de baixo desempenho" },
		},
		"default"
	},
	{
		"dosbox_pure_auto_target",
		"Avançado > Uso da CPU de baixa latência", NULL,
		"No modo de baixa latência, ao emular o DOS o mais rápido possível, quanto tempo por quadro deve ser usado pela emulação." "\n"
		"Se o vídeo estiver travando, diminua isso ou melhore o desempenho de renderização no frontend (por exemplo, desativando o vsync ou processamento de vídeo)." "\n"
		"Use as estatísticas de desempenho para encontrar facilmente o máximo que ainda atinge a taxa de quadros alvo emulada." "\n\n", NULL, //Fim da seção Emulação > Avançado
		"Emulação",
		{
			//{ "0.2", "20%" }, { "0.21", "21%" }, { "0.22", "22%" }, { "0.23", "23%" }, { "0.24", "24%" }, { "0.25", "25%" }, { "0.26", "26%" }, { "0.27", "27%" }, { "0.28", "28%" }, { "0.29", "29%" },
			//{ "0.3", "30%" }, { "0.31", "31%" }, { "0.32", "32%" }, { "0.33", "33%" }, { "0.34", "34%" }, { "0.35", "35%" }, { "0.36", "36%" }, { "0.37", "37%" }, { "0.38", "38%" }, { "0.39", "39%" },
			//{ "0.4", "40%" }, { "0.41", "41%" }, { "0.42", "42%" }, { "0.43", "43%" }, { "0.44", "44%" }, { "0.45", "45%" }, { "0.46", "46%" }, { "0.47", "47%" }, { "0.48", "48%" }, { "0.49", "49%" },
			{ "0.5", "50%" }, { "0.51", "51%" }, { "0.52", "52%" }, { "0.53", "53%" }, { "0.54", "54%" }, { "0.55", "55%" }, { "0.56", "56%" }, { "0.57", "57%" }, { "0.58", "58%" }, { "0.59", "59%" },
			{ "0.6", "60%" }, { "0.61", "61%" }, { "0.62", "62%" }, { "0.63", "63%" }, { "0.64", "64%" }, { "0.65", "65%" }, { "0.66", "66%" }, { "0.67", "67%" }, { "0.68", "68%" }, { "0.69", "69%" },
			{ "0.7", "70%" }, { "0.71", "71%" }, { "0.72", "72%" }, { "0.73", "73%" }, { "0.74", "74%" }, { "0.75", "75%" }, { "0.76", "76%" }, { "0.77", "77%" }, { "0.78", "78%" }, { "0.79", "79%" },
			{ "0.8", "80%" }, { "0.81", "81%" }, { "0.82", "82%" }, { "0.83", "83%" }, { "0.84", "84%" }, { "0.85", "85%" }, { "0.86", "86%" }, { "0.87", "87%" }, { "0.88", "88%" }, { "0.89", "89%" },
			{ "0.9", "90%" }, { "0.91", "91%" }, { "0.92", "92%" }, { "0.93", "93%" }, { "0.94", "94%" }, { "0.95", "95%" }, { "0.96", "96%" }, { "0.97", "97%" }, { "0.98", "98%" }, { "0.99", "99%" },
			{ "1.0", "100%" },
		},
		"0.9",
	},

	// Input
	{
		"dosbox_pure_on_screen_keyboard",
		"Ativar Teclado na Tela", NULL,
		"Ative a funcionalidade de Teclado na Tela, que pode ser ativada com o botão L3 no controle.", NULL,
		"Entrada",
		{ { "true", "Ativado" }, { "false", "Desativado" } },
		"true"
	},
	{
		"dosbox_pure_mouse_input",
		"Modo de Entrada do Mouse", NULL,
		"Você pode desativar o tratamento de entrada de um mouse ou uma tela sensível ao toque (o mouse emulado através do joystick ainda funcionará)." "\n"
		"No modo de touchpad, use o arrastar para mover, toque para clicar, toque com dois dedos para clicar com o botão direito e pressione e segure para arrastar", NULL,
		"Entrada",
		{
#if defined(ANDROID) || defined(DBP_IOS) || defined(HAVE_LIBNX) || defined(_3DS) || defined(WIIU) || defined(VITA)
			{ "pad", "Modo touchpad (padrão, ver descrição, melhor para telas sensíveis ao toque)" },
			{ "direct", "Mouse controlado diretamente (não suportado por todos os jogos)" },
			{ "true", "Mouse virtual" },
#else
		{ "true", "Mouse virtual (padrão)" },
		{ "direct", "Mouse controlado diretamente (não suportado por todos os jogos)" },
		{ "pad", "Modo touchpad (consulte a descrição, melhor para telas sensíveis ao toque)" },
#endif
		{ "false", "Desativado (ignorar entradas do mouse)" },
		},
		"true"
	},
	{
		"dosbox_pure_mouse_wheel",
		"Vincular Roda do Mouse a Tecla", NULL,
		"Vincule a roda do mouse para cima e para baixo a duas teclas do teclado para poder usá-la em jogos de DOS.", NULL,
		"Entrada",
		{
			{ "67/68", "Colchete Esquerdo/Colchete Direito" },
			{ "72/71", "Vírgula/Ponto" },
			{ "79/82", "Page-Up/Page-Down" },
			{ "78/81", "Início/Fim" },
			{ "80/82", "Delete/Page-Down" },
			{ "64/65", "Hífen/Igual" },
			{ "69/70", "Ponto e Vírgula/Aspas" },
			{ "99/100", "Menos do Teclado Numérico/Mais do Teclado Numérico" },
			{ "97/98", "Dividir do Teclado Numérico/Multiplicar do Teclado Numérico" },
			{ "84/85", "Cima/Baixo" },
			{ "83/86", "Esquerda/Direita" },
			{ "11/13", "Q/E" },
			{ "none", "Desativar" },
		},
		"67/68"
	},
	{
		"dosbox_pure_mouse_speed_factor",
		"Sensibilidade do Mouse", NULL,
		"Define a velocidade geral de movimento do cursor do mouse." "\n\n", NULL, //end of Input section
		"Entrada",
		{
			{ "0.2",  "20%" }, { "0.25",  "25%" }, { "0.3",  "30%" }, { "0.35",  "35%" }, { "0.4",  "40%" }, { "0.45",  "45%" },
			{ "0.5",  "50%" }, { "0.55",  "55%" }, { "0.6",  "60%" }, { "0.65",  "65%" }, { "0.7",  "70%" }, { "0.75",  "75%" },
			{ "0.8",  "80%" }, { "0.85",  "85%" }, { "0.9",  "90%" }, { "0.95",  "95%" }, { "1.0", "100%" }, { "1.1" , "110%" },
			{ "1.2", "120%" }, { "1.3" , "130%" }, { "1.4", "140%" }, { "1.5" , "150%" }, { "1.6", "160%" }, { "1.7" , "170%" },
			{ "1.8", "180%" }, { "1.9" , "190%" }, { "2.0", "200%" }, { "2.2" , "220%" }, { "2.4", "240%" }, { "2.6" , "260%" },
			{ "2.8", "280%" }, { "3.0" , "300%" }, { "3.2", "320%" }, { "3.4" , "340%" }, { "3.6", "360%" }, { "3.8" , "380%" },
			{ "4.0", "400%" }, { "4.2" , "420%" }, { "4.4", "440%" }, { "4.6",  "460%" }, { "4.8", "480%" }, { "5.0",  "500%" },
		},
		"1.0"
	},
	{
		"dosbox_pure_mouse_speed_factor_x",
		"Avançado > Sensibilidade do Mouse Horizontal.", NULL,
		"Experimente com este valor se o mouse estiver muito rápido/lento ao se mover para a esquerda/direita.", NULL,
		"Entrada",
		{
			{ "0.2",  "20%" }, { "0.25",  "25%" }, { "0.3",  "30%" }, { "0.35",  "35%" }, { "0.4",  "40%" }, { "0.45",  "45%" },
			{ "0.5",  "50%" }, { "0.55",  "55%" }, { "0.6",  "60%" }, { "0.65",  "65%" }, { "0.7",  "70%" }, { "0.75",  "75%" },
			{ "0.8",  "80%" }, { "0.85",  "85%" }, { "0.9",  "90%" }, { "0.95",  "95%" }, { "1.0", "100%" }, { "1.1" , "110%" },
			{ "1.2", "120%" }, { "1.3" , "130%" }, { "1.4", "140%" }, { "1.5" , "150%" }, { "1.6", "160%" }, { "1.7" , "170%" },
			{ "1.8", "180%" }, { "1.9" , "190%" }, { "2.0", "200%" }, { "2.2" , "220%" }, { "2.4", "240%" }, { "2.6" , "260%" },
			{ "2.8", "280%" }, { "3.0" , "300%" }, { "3.2", "320%" }, { "3.4" , "340%" }, { "3.6", "360%" }, { "3.8" , "380%" },
			{ "4.0", "400%" }, { "4.2" , "420%" }, { "4.4", "440%" }, { "4.6",  "460%" }, { "4.8", "480%" }, { "5.0",  "500%" },
		},
		"1.0"
	},
	{
		"dosbox_pure_actionwheel_inputs",
		"Avançado > Entradas da Roda de Ação", NULL,
		"Define quais entradas controlam a roda de ação.", NULL,
		"Entrada",
		{
			{ "14", "Alavanca Direita, D-Pad, Mouse (Padrão)" }, { "6",  "Alavanca Direita, D-Pad" }, { "10", "Alavanca Direita, Mouse" }, { "2",  "Alavanca Direita" },
			{ "15", "Ambas as Alavancas, D-Pad, Mouse" }, { "7",  "Ambas as Alavancas, D-Pad" }, { "11", "Ambas as Alavancas, Mouse" }, { "3",  "Ambas as Alavancas" },
			{ "13", "Alavanca Esquerda, D-Pad, Mouse" }, { "5",  "Alavanca Esquerda, D-Pad" }, { "9",  "Alavanca Esquerda, Mouse" }, { "1",  "Alavanca Esquerda" },
			{ "12", "D-Pad, Mouse" }, { "4",  "D-Pad" }, { "8",  "Mouse" },
		},
		"14"
	},
	{
		"dosbox_pure_auto_mapping",
		"Avançado > Mapeamentos Automáticos de Gamepad", NULL,
		"O DOSBox Pure pode aplicar automaticamente um esquema de mapeamento de controle de gamepad quando detecta um jogo." "\n"
		"Esses mapeamentos de botões são fornecidos pelo Projeto Keyb2Joypad (de Jemy Murphy e bigjim).", NULL,
		"Entrada",
		{ { "true", "Ativado (padrão)" }, { "notify", "Ativar com notificação na detecção de jogo" }, { "false", "Desativado" } },
		"true"
	},
	{
		"dosbox_pure_keyboard_layout",
		"Avançado > Layout do Teclado", NULL,
		"Selecione o layout do teclado (não afetará o Teclado na Tela).", NULL,
		"Entrada",
		{
			{ "us",    "EUA" },
			{ "uk",    "Reino Unido" },
			{ "be",    "Bélgica" },
			{ "br",    "Brasil (padrão)" },
			{ "hr",    "Croácia" },
			{ "cz243", "República Tcheca" },
			{ "dk",    "Dinamarca" },
			{ "su",    "Finlândia" },
			{ "fr",    "França" },
			{ "gr",    "Alemanha" },
			{ "gk",    "Grécia" },
			{ "hu",    "Hungria" },
			{ "is161", "Islândia" },
			{ "it",    "Itália" },
			{ "nl",    "Holanda" },
			{ "no",    "Noruega" },
			{ "pl",    "Polônia" },
			{ "po",    "Portugal" },
			{ "ru",    "Rússia" },
			{ "sk",    "Eslováquia" },
			{ "si",    "Eslovênia" },
			{ "sp",    "Espanha" },
			{ "sv",    "Suécia" },
			{ "sg",    "Suíça (Alemão)" },
			{ "sf",    "Suíça (Francês)" },
			{ "tr",    "Turquia" },
		},
		"br"
	},
	{
		"dosbox_pure_menu_transparency",
		"Avançado > Transparência do Menu", NULL,
		"Defina o nível de transparência do Teclado na Tela e do Mapeador de Controle.", NULL,
		"Entrada",
		{
			{ "10", "10%" }, { "20", "20%" }, { "30", "30%" }, { "40", "40%" }, { "50", "50%" }, { "60", "60%" }, { "70", "70%" }, { "80", "80%" }, { "90", "90%" }, { "100", "100%" },
		},
		"70"
	},
	{
		"dosbox_pure_joystick_analog_deadzone",
		"Avançado > Zona Morta do Analógica do Joystick", NULL,
		"Defina a zona morta das alavancas analógicas do joystick. Pode ser usada para eliminar desvios causados por hardware de joystick mal calibrado.", NULL,
		"Entrada",
		{
			{ "0",  "0%" }, { "5",  "5%" }, { "10", "10%" }, { "15", "15%" }, { "20", "20%" }, { "25", "25%" }, { "30", "30%" }, { "35", "35%" }, { "40", "40%" },
		},
		"15"
	},
	{
		"dosbox_pure_joystick_timed",
		"Avançado > Habilitar Intervalos Cronometrados do Joystick", NULL,
		"Habilitar intervalos cronometrados para os eixos do joystick. Experimente esta opção se o seu joystick apresentar desvio." "\n\n", NULL, //end of Input > Advanced section
		"Entrada",
		{ { "true", "Ativado (padrão)" }, { "false", "Desativado" } },
		"true"
	},

	// Performance
	{
		"dosbox_pure_cycles",
		"Desempenho Emulado", NULL,
		"O desempenho bruto que o DOSBox tentará emular." "\n\n", NULL, //end of Performance section
		"Desempenho",
		{
			{ "auto",    "AUTO - O DOSBox tentará detectar as necessidades de desempenho (padrão)" },
			{ "max",     "MAX - Emular o maior número de instruções possível" },
			{ "315",     "8086/8088, 4,77 MHz a partir de 1980 (315 cps)" },
			{ "1320",    "286, 6 MHz a partir de 1982 (1320 cps)" },
			{ "2750",    "286, 12,5 MHz a partir de 1985 (2750 cps)" },
			{ "4720",    "386, 20 MHz a partir de 1987 (4720 cps)" },
			{ "7800",    "386DX, 33 MHz a partir de 1989 (7800 cps)" },
			{ "13400",   "486DX, 33 MHz a partir de 1990 (13400 cps)" },
			{ "26800",   "486DX2, 66 MHz a partir de 1992 (26800 cps)" },
			{ "77000",   "Pentium, 100 MHz a partir de 1995 (77000 cps)" },
			{ "200000",  "Pentium II, 300 MHz a partir de 1997 (200000 cps)" },
			{ "500000",  "Pentium III, 600 MHz a partir de 1999 (500000 cps)" },
			{ "1000000", "AMD Athlon, 1,2 GHz a partir de 2000 (1000000 cps)" },
		},
		"auto"
	},
	{
		"dosbox_pure_cycles_max",
		"Detalhado > Desempenho Emulado Máximo", NULL,
		"Com a velocidade de CPU dinâmica (AUTO ou MÁX acima), o nível máximo de desempenho emulado.", NULL,
		"Desempenho",
		{
			{ "none",    "Ilimitado" },
			{ "315",     "8086/8088, 4.77 MHz de 1980 (315 cps)" },
			{ "1320",    "286, 6 MHz de 1982 (1320 cps)" },
			{ "2750",    "286, 12.5 MHz de 1985 (2750 cps)" },
			{ "4720",    "386, 20 MHz de 1987 (4720 cps)" },
			{ "7800",    "386DX, 33 MHz de 1989 (7800 cps)" },
			{ "13400",   "486DX, 33 MHz de 1990 (13400 cps)" },
			{ "26800",   "486DX2, 66 MHz de 1992 (26800 cps)" },
			{ "77000",   "Pentium, 100 MHz de 1995 (77000 cps)" },
			{ "200000",  "Pentium II, 300 MHz de 1997 (200000 cps)" },
			{ "500000",  "Pentium III, 600 MHz de 1999 (500000 cps)" },
			{ "1000000", "AMD Athlon, 1.2 GHz de 2000 (1000000 cps)" },
		},
		"auto"
	},
	{
		"dosbox_pure_cycles_scale",
		"Detalhado > Escala de Desempenho", NULL,
		"Ajuste fino do desempenho emulado para necessidades específicas.", NULL,
		"Desempenho",
		{
			{ "0.2",  "20%" }, { "0.25",  "25%" }, { "0.3",  "30%" }, { "0.35",  "35%" }, { "0.4",  "40%" }, { "0.45",  "45%" },
			{ "0.5",  "50%" }, { "0.55",  "55%" }, { "0.6",  "60%" }, { "0.65",  "65%" }, { "0.7",  "70%" }, { "0.75",  "75%" },
			{ "0.8",  "80%" }, { "0.85",  "85%" }, { "0.9",  "90%" }, { "0.95",  "95%" }, { "1.0", "100%" }, { "1.05", "105%" },
			{ "1.1", "110%" }, { "1.15", "115%" }, { "1.2", "120%" }, { "1.25", "125%" }, { "1.3", "130%" }, { "1.35", "135%" },
			{ "1.4", "140%" }, { "1.45", "145%" }, { "1.5", "150%" }, { "1.55", "155%" }, { "1.6", "160%" }, { "1.65", "165%" },
			{ "1.7", "170%" }, { "1.75", "175%" }, { "1.8", "180%" }, { "1.85", "185%" }, { "1.9", "190%" }, { "1.95", "195%" },
			{ "2.0", "200%" },
		},
		"1.0",
	},
	{
		"dosbox_pure_cycle_limit",
		"Detalhado > Limitar Uso da CPU", NULL,
		"Quanto tempo por quadro deve ser usado pela emulação ao emular o DOS o mais rápido possível." "\n"
		"Diminua isso se o seu dispositivo esquentar enquanto usa este núcleo." "\n\n", NULL, //end of Performance > Detailed section
		"Desempenho",
		{
			//{ "0.2", "20%" }, { "0.21", "21%" }, { "0.22", "22%" }, { "0.23", "23%" }, { "0.24", "24%" }, { "0.25", "25%" }, { "0.26", "26%" }, { "0.27", "27%" }, { "0.28", "28%" }, { "0.29", "29%" },
			//{ "0.3", "30%" }, { "0.31", "31%" }, { "0.32", "32%" }, { "0.33", "33%" }, { "0.34", "34%" }, { "0.35", "35%" }, { "0.36", "36%" }, { "0.37", "37%" }, { "0.38", "38%" }, { "0.39", "39%" },
			//{ "0.4", "40%" }, { "0.41", "41%" }, { "0.42", "42%" }, { "0.43", "43%" }, { "0.44", "44%" }, { "0.45", "45%" }, { "0.46", "46%" }, { "0.47", "47%" }, { "0.48", "48%" }, { "0.49", "49%" },
			{ "0.5", "50%" }, { "0.51", "51%" }, { "0.52", "52%" }, { "0.53", "53%" }, { "0.54", "54%" }, { "0.55", "55%" }, { "0.56", "56%" }, { "0.57", "57%" }, { "0.58", "58%" }, { "0.59", "59%" },
			{ "0.6", "60%" }, { "0.61", "61%" }, { "0.62", "62%" }, { "0.63", "63%" }, { "0.64", "64%" }, { "0.65", "65%" }, { "0.66", "66%" }, { "0.67", "67%" }, { "0.68", "68%" }, { "0.69", "69%" },
			{ "0.7", "70%" }, { "0.71", "71%" }, { "0.72", "72%" }, { "0.73", "73%" }, { "0.74", "74%" }, { "0.75", "75%" }, { "0.76", "76%" }, { "0.77", "77%" }, { "0.78", "78%" }, { "0.79", "79%" },
			{ "0.8", "80%" }, { "0.81", "81%" }, { "0.82", "82%" }, { "0.83", "83%" }, { "0.84", "84%" }, { "0.85", "85%" }, { "0.86", "86%" }, { "0.87", "87%" }, { "0.88", "88%" }, { "0.89", "89%" },
			{ "0.9", "90%" }, { "0.91", "91%" }, { "0.92", "92%" }, { "0.93", "93%" }, { "0.94", "94%" }, { "0.95", "95%" }, { "0.96", "96%" }, { "0.97", "97%" }, { "0.98", "98%" }, { "0.99", "99%" },
			{ "1.0", "100%" },
		},
		"1.0",
	},

	// Video
	{
		"dosbox_pure_machine",
		"Chip Gráfico Emulado (necessário reiniciar)", NULL,
		"O tipo de chip gráfico que o DOSBox emulará.", NULL,
		"Vídeo",
		{
			{ "svga",     "SVGA (Super Video Graphics Array) (padrão)" },
			{ "vga",      "VGA (Video Graphics Array)" },
			{ "ega",      "EGA (Enhanced Graphics Adapter)" },
			{ "cga",      "CGA (Color Graphics Adapter)" },
			{ "tandy",    "Tandy (Tandy Graphics Adapter)" },
			{ "hercules", "Hercules (Hercules Graphics Card)" },
			{ "pcjr",     "PCjr" },
		},
		"svga"
	},
	{
		"dosbox_pure_cga",
		"CGA Mode", NULL,
		"A variação de CGA que está sendo emulada.", NULL,
		"Vídeo",
		{
			{ "early_auto", "Modelo antigo, modo composto automático (padrão)" },
			{ "early_on",   "Modelo antigo, modo composto ligado" },
			{ "early_off",  "Modelo antigo, modo composto desligado" },
			{ "late_auto", "Modelo recente, modo composto automático" },
			{ "late_on",   "Modelo recente, modo composto ligado" },
			{ "late_off",  "Modelo recente, modo composto desligado" },
		},
		"early_auto"
	},
	{
		"dosbox_pure_hercules",
		"Modo de Cor para Hercules", NULL,
		"O esquema de cores para a emulação Hercules.", NULL,
		"Vídeo",
		{
			{ "white", "Preto e branco (padrão)" },
			{ "amber", "Preto e âmbar" },
			{ "green", "Preto e verde" },
		},
		"white"
	},
	{
		"dosbox_pure_svga",
		"Modo SVGA (necessário reiniciar)", NULL,
		"A variação SVGA que está sendo emulada. Tente mudar isso se encontrar problemas gráficos.", NULL,
		"Vídeo",
		{
			{ "svga_s3",       "S3 Trio64 (padrão)" },
			{ "vesa_nolfb",    "S3 Trio64 sem hack de buffer de linha (reduz cintilação em alguns jogos)" },
			{ "vesa_oldvbe",   "S3 Trio64 VESA 1.3" },
			{ "svga_et3000",   "Tseng Labs ET3000" },
			{ "svga_et4000",   "Tseng Labs ET4000" },
			{ "svga_paradise", "Paradise PVGA1A" },
		},
		"svga_s3"
	},
	{
		"dosbox_pure_svgamem",
		"Memória SVGA (necessária reinicialização)", NULL,
		"A quantidade de memória disponível para a placa SVGA emulada.", NULL,
		"Vídeo",
		{
			{ "0",  "512KB" },
			{ "1", "1MB" },
			{ "2", "2MB (padrão)" },
			{ "3", "3MB" },
			{ "4", "4MB" },
			{ "8", "8MB (nem sempre reconhecido)" },
		},
		"2"
	},
	{
		"dosbox_pure_voodoo",
		"Emulação 3dfx Voodoo", NULL,
		"Habilita certos jogos com suporte para o acelerador 3D Voodoo." "\n"
		"Emulador 3dfx Voodoo Graphics SST-1/2 por Aaron Giles e a equipe do MAME (licença: BSD-3-Clause)", NULL,
		"Vídeo",
		{
			{ "8mb", "Habilitado - 8MB de memória (padrão)" },
			{ "12mb", "Habilitado - 12MB de memória, Textura Dual" },
			{ "4mb", "Habilitado - 4MB de memória, Somente Baixa Resolução" },
			{ "off", "Desabilitado" },
		},
		"8mb",
	},
	{
		"dosbox_pure_voodoo_perf",
		"Desempenho 3dfx Voodoo", NULL,
		"Opções para ajustar o comportamento da emulação 3dfx Voodoo." "\n"
		"Mudar para OpenGL requer um reinício." "\n"
		"Se o OpenGL estiver disponível, a aceleração 3D do lado do anfitrião é utilizada, o que pode tornar a renderização 3D muito mais rápida.", NULL,
		"Vídeo",
		{
			{ "1", "Multi Threaded de Software (padrão)" },
			{ "4", "Hardware OpenGL" },
			{ "3", "Multi Threaded de Software, baixa qualidade" },
			{ "2", "Single Threaded de Software, baixa qualidade" },
			{ "0", "Single Threaded de Software" },
		},
		"1",
	},
	{
		"dosbox_pure_voodoo_scale",
		"3dfx Voodoo Escalonamento OpenGL", NULL,
		"Aumentar a resolução nativa da imagem renderizada.", NULL,
		"Vídeo",
		{
			{ "1", "1x" }, { "2", "2x" }, { "3", "3x" }, { "4", "4x" }, { "5", "5x" }, { "6", "6x" }, { "7", "7x" }, { "8", "8x" },
		},
		"1",
	},
	{
		"dosbox_pure_voodoo_gamma",
		"Correção de Gama 3dfx Voodoo", NULL,
		"Mudar o brilho da saída renderizada 3dfx.", NULL,
		"Vídeo",
		{
			{ "-10", "-10" }, { "-9", "-9" }, { "-8", "-8" }, { "-7", "-7" }, { "-6", "-6" }, { "-5", "-5" }, { "-4", "-4" }, { "-3", "-3" }, { "-2", "-2" }, { "-1", "-1" },
			{ "0", "Nenhum" },
			{ "1", "+1" }, { "2", "+2" }, { "3", "+3" }, { "4", "+4" }, { "5", "+5" }, { "6", "+6" }, { "7", "+7" }, { "8", "+8" }, { "9", "+9" }, { "10", "+10" },
			{ "999", "Desativar Correção de Gama" },
		},
		"-2",
	},
	{
		"dosbox_pure_aspect_correction",
		"Correção da Proporção de Tela", NULL,
		"Quando ativada, a proporção de tela do núcleo é ajustada para o que um monitor CRT exibiria.", NULL,
		"Vídeo",
		{
			{ "false", "Desativado (padrão)" },
			{ "true", "Ativado (escaneamento único)" },
			{ "doublescan", "Ativado (escaneamento duplo quando aplicável)" },
			{ "padded", "Ajustado para 4:3 (escaneamento único)" },
			{ "padded-doublescan", "Ajustado para 4:3 (escaneamento duplo quando aplicável)" },
		},
		"true"
	},
	{
		"dosbox_pure_overscan",
		"Tamanho da Borda do Overscan", NULL,
		"Quando habilitado, mostra uma borda ao redor da tela. Alguns jogos usam a cor da borda para transmitir informações." "\n\n", NULL, // fim da seção de Vídeo
		"Vídeo",
		{ { "0", "Desativado (padrão)" }, { "1", "Pequeno" }, { "2", "Médio" }, { "3", "Grande" } },
		"0"
	},

	// System
	{
		"dosbox_pure_memory_size",
		"Tamanho da Memória (necessário reiniciar)", NULL,
		"A quantidade de memória (alta) que a máquina emulada possui. Você também pode desativar a memória estendida (EMS/XMS)." "\n"
		"Não é recomendado usar mais do que o padrão devido à incompatibilidade com certos jogos e aplicativos.", NULL,
		"Sistema",
		{
			{ "none", "Desativar memória estendida (sem EMS/XMS)" },
			{ "4",  "4 MB" },
			{ "8",  "8 MB" },
			{ "16", "16 MB (padrão)" },
			{ "24", "24 MB" },
			{ "32", "32 MB" },
			{ "48", "48 MB" },
			{ "64", "64 MB" },
			{ "96", "96 MB" },
			{ "128", "128 MB" },
			{ "224", "224 MB" },
			{ "256", "256 MB" },
			{ "512", "512 MB" },
			{ "1024", "1024 MB" },
		},
		"16"
	},
	{
		"dosbox_pure_modem",
		"Tipo de Modem", NULL,
		"Tipo de modem emulado em COM1 para jogo em rede. Com o modem dial-up, um lado precisa discar qualquer número para se conectar.", NULL,
		"Sistema",
		{
			{ "null", "Modem Nulo (Serial Direto)" },
			{ "dial", "Modem Dial-Up (Padrão Hayes)" },
		},
		"null"
	},
	{
		"dosbox_pure_cpu_type",
		"Tipo de CPU (necessário reiniciar)", NULL,
		"Tipo de CPU emulado. 'Auto' é a opção mais rápida." "\n"
			"Jogos que requerem seleção específica de tipo de CPU:" "\n"
			"386 (pré-busca): X-Men: Madness in The Murderworld, Terminator 1, Contra, Fifa International Soccer 1994" "\n"
			"486 (lento): Betrayal in Antara" "\n"
			"Pentium (lento): Fifa International Soccer 1994, jogos do Windows 95/Windows 3.x" "\n\n", NULL, //end of System section
		"Sistema",
		{
			{ "auto", "Automático - Conjunto de recursos misto com máxima performance e compatibilidade" },
			{ "386", "386 - Conjunto de instruções 386 com acesso rápido à memória" },
			{ "386_slow", "386 (lento) - Conjunto de instruções 386 com verificações de privilégio de memória" },
			{ "386_prefetch", "386 (pré-busca) - Com emulação de fila de pré-busca (apenas nos núcleos 'auto' e 'normal')" },
			{ "486_slow", "486 (lento) - Conjunto de instruções 486 com verificações de privilégio de memória" },
			{ "pentium_slow", "Pentium (lento) - Conjunto de instruções 586 com verificações de privilégio de memória" },
		},
		"auto"
	},
	{
		"dosbox_pure_cpu_core",
		"Avançado > Núcleo da CPU", NULL,
		"Método de emulação (núcleo da CPU do DOSBox) usado.", NULL,
		"Sistema",
		{
			#if defined(C_DYNAMIC_X86)
			{ "auto", "Automático - Jogos em modo real usam normal, jogos em modo protegido usam dinâmico" },
			{ "dynamic", "Dinâmico - Recompilação dinâmica (rápida, usando a implementação dynamic_x86)" },
			#elif defined(C_DYNREC)
			{ "auto", "Automático - Jogos em modo real usam normal, jogos em modo protegido usam dinâmico" },
			{ "dynamic", "Dinâmico - Recompilação dinâmica (rápida, usando a implementação dynrec)" },
			#endif
			{ "normal", "Normal (interpretador)" },
			{ "simple", "Simples (interpretador otimizado para jogos antigos em modo real)" },
		},
		#if defined(C_DYNAMIC_X86) || defined(C_DYNREC)
		"auto"
		#else
		"normal"
		#endif
	},
	{
		"dosbox_pure_bootos_ramdisk",
		"Avançado > Modificações no Disco do SO (necessário reiniciar)", NULL,
		"Ao executar um sistema operacional instalado, as modificações na unidade C: serão feitas na imagem de disco por padrão." "\n"
		"Definir para 'Descartar' permite que o conteúdo seja fechado a qualquer momento sem preocupações com corrupção do sistema de arquivos ou do registro." "\n"
		"Ao usar 'Salvar Diferença por Conteúdo', a imagem do disco nunca deve ser modificada novamente, caso contrário, as diferenças existentes se tornarão inutilizáveis.", NULL,
		"Sistema",
		{
			{ "false", "Manter (padrão)" },
			{ "true", "Descartar" },
			{ "diff", "Salvar Diferença por Conteúdo" },
		},
		"false"
	},
	{
		"dosbox_pure_bootos_dfreespace",
		"Avançado > Espaço Livre em D: no SO (necessário reiniciar)", NULL,
		"Controla a quantidade de espaço livre disponível na unidade D: ao executar um sistema operacional instalado." "\n"
		"Se o tamanho total da unidade D: (dados + espaço livre) exceder 2 GB, não poderá ser usado nas versões anteriores do Windows 95." "\n"
		"ATENÇÃO: Os arquivos de salvamento criados estão vinculados a essa configuração, portanto, alterá-la ocultará todas as alterações existentes na unidade D:.", NULL,
		"Sistema",
		{ { "1024", "1GB (padrão)" }, { "2048", "2GB" }, { "4096", "4GB" }, { "8192", "8GB" } },
		"1024"
	},
	{
		"dosbox_pure_bootos_forcenormal",
		"Avançado > Forçar Núcleo Normal no SO", NULL,
		"O núcleo normal pode ser mais estável ao executar um sistema operacional instalado." "\n"
		"Isso pode ser ligado e desligado para contornar travamentos." "\n\n", NULL, //end of System > Advanced section
		"Sistema",
		{ { "false", "Desativado (padrão)" }, { "true", "On" } },
		"false"
	},

	// Audio
	{
		"dosbox_pure_audiorate",
		"Taxa de Amostragem de Áudio (necessário reiniciar)", NULL,
		"Isso deve corresponder à configuração de taxa de saída de áudio do frontend (Hz).", NULL,
		"Áudio",
		{
			{ "48000", NULL },
			{ "44100", NULL },
			#ifdef _3DS
			{ "32730", NULL },
			#endif
			{ "32000", NULL },
			{ "22050", NULL },
			{ "16000", NULL },
			{ "11025", NULL },
			{  "8000", NULL },
			{ "49716", NULL }, //for perfect OPL emulation
		},
		DBP_DEFAULT_SAMPLERATE_STRING
	},
	{
		"dosbox_pure_sblaster_conf",
		"Configurações do SoundBlaster", NULL,
		"Defina o endereço, interrupção, DMA de 8 bits baixos e DMA de 16 bits altos.", NULL,
		"Áudio",
		{
			// Some common (and less common) port, irq, low and high dma settings (based on a very scientific web search)
			{ "A220 I7 D1 H5",  "Porta 0x220, IRQ 7, 8-Bit DMA 1, 16-bit DMA 5"  },
			{ "A220 I5 D1 H5",  "Porta 0x220, IRQ 5, 8-Bit DMA 1, 16-bit DMA 5"  },
			{ "A240 I7 D1 H5",  "Porta 0x240, IRQ 7, 8-Bit DMA 1, 16-bit DMA 5"  },
			{ "A240 I7 D3 H7",  "Porta 0x240, IRQ 7, 8-Bit DMA 3, 16-bit DMA 7"  },
			{ "A240 I2 D3 H7",  "Porta 0x240, IRQ 2, 8-Bit DMA 3, 16-bit DMA 7"  },
			{ "A240 I5 D3 H5",  "Porta 0x240, IRQ 5, 8-Bit DMA 3, 16-bit DMA 5"  },
			{ "A240 I5 D1 H5",  "Porta 0x240, IRQ 5, 8-Bit DMA 1, 16-bit DMA 5"  },
			{ "A240 I10 D3 H7", "Porta 0x240, IRQ 10, 8-Bit DMA 3, 16-bit DMA 7" },
			{ "A280 I10 D0 H6", "Porta 0x280, IRQ 10, 8-Bit DMA 0, 16-bit DMA 6" },
			{ "A280 I5 D1 H5",  "Porta 0x280, IRQ 5, 8-Bit DMA 1, 16-bit DMA 5"  },
		},
		"A220 I7 D1 H5"
	},
	{
		"dosbox_pure_midi",
		"Saída MIDI", NULL,
		"Selecione o arquivo SoundFont .SF2, arquivo .ROM ou interface usada para saída MIDI." "\n"
		"Para adicionar SoundFonts ou arquivos ROM, copie-os para o diretório 'system' do frontend." "\n"
		"Para usar o driver MIDI do frontend, certifique-se de que ele esteja configurado corretamente." "\n\n", NULL, //end of Audio section
		"Áudio",
		{
			// dynamically filled in retro_init
		},
		"disabled"
	},
	{
		"dosbox_pure_sblaster_type",
		"Avançado > Tipo de SoundBlaster", NULL,
		"Tipo de placa SoundBlaster emulada.", NULL,
		"Áudio",
		{
			{ "sb16", "SoundBlaster 16 (padrão)" },
			{ "sbpro2", "SoundBlaster Pro 2" },
			{ "sbpro1", "SoundBlaster Pro" },
			{ "sb2", "SoundBlaster 2.0" },
			{ "sb1", "SoundBlaster 1.0" },
			{ "gb", "GameBlaster" },
			{ "none", "nenhum" },
		},
		"sb16"
	},
	{
		"dosbox_pure_sblaster_adlib_mode",
		"Avançado > Modo Adlib/FM do SoundBlaster", NULL,
		"O modo de síntese FM emulado pelo SoundBlaster. Todos os modos são compatíveis com o Adlib, exceto o CMS.", NULL,
		"Áudio",
		{
			{ "auto",     "Automático (selecionar com base no tipo de SoundBlaster) (padrão)" },
			{ "cms",      "CMS (Creative Music System / GameBlaster)" },
			{ "opl2",     "OPL-2 (AdLib / OPL-2 / Yamaha 3812)" },
			{ "dualopl2", "Dual OPL-2 (Dual OPL-2 usado pelo SoundBlaster Pro 1.0 para som estéreo)" },
			{ "opl3",     "OPL-3 (AdLib / OPL-3 / Yamaha YMF262)" },
			{ "opl3gold", "OPL-3 Gold (AdLib Gold / OPL-3 / Yamaha YMF262)" },
			{ "none",     "Desativado" },
		},
		"auto"
	},
	{
		"dosbox_pure_sblaster_adlib_emu",
		"Avançado > Provedor de Adlib SoundBlaster", NULL,
		"Provedor para a emulação do Adlib. O padrão possui boa qualidade e baixos requisitos de desempenho.", NULL,
"		Áudio",
		{
			{ "default", "Padrão" },
			{ "nuked", "Alta qualidade Nuked OPL3" },
		},
		"default"
	},
	{
		"dosbox_pure_gus",
		"Avançado > Habilitar Emulação do Gravis Ultrasound (necessário reiniciar)", NULL,
		"Habilitar emulação do Gravis Ultrasound. As configurações estão fixadas em porta 0x240, IRQ 5, DMA 3." "\n"
		"Se a variável ULTRADIR precisar ser diferente do padrão 'C:\\ULTRASND' você precisará inserir 'SET ULTRADIR=...' na linha de comando ou em um arquivo em lote." "\n\n", NULL, // fim da seção de Áudio > Avançado
		"Áudio",
		{ { "false", "Desativado (padrão)" }, { "true", "Ativado" } },
		"false"
	},
	{
		"dosbox_pure_tandysound",
		"Avançado > Habilitar Dispositivo de Som Tandy (reinício necessário)", NULL,
		"Habilita a emulação do Dispositivo de Som Tandy mesmo quando executando sem a emulação do Adaptador Gráfico Tandy.", NULL,
		"Áudio",
		{ { "auto", "Desativado (padrão)" }, { "on", "Ativado" } },
		"auto"
	},
	{
		"dosbox_pure_swapstereo",
		"Avançado > Trocar canais estéreo", NULL,
		"Trocar o canal de áudio esquerdo e direito." "\n\n", NULL, //end of Audio > Advanced section
		"Áudio",
		{ { "false", "Desativado (padrão)" }, { "true", "Ativado" } },
		"false"
	},

	{ NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL }
};
