/*
 *  Copyright (C) 2020-2025 Bernhard Schelling
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

namespace DBP_OptionCat
{
	static const char* General     = "Geral";
	static const char* Input       = "Entrada";
	static const char* Performance = "Desempenho";
	static const char* Video       = "Video";
	static const char* System      = "Sistema";
	static const char* Audio       = "Audio";
};

static retro_core_option_v2_category option_cats[] =
{
	#ifndef DBP_STANDALONE
	{ DBP_OptionCat::General,     DBP_OptionCat::General,     "Configuracoes gerais (estados de salvamento, menu iniciar, FPS fixo)." },
	#else
	{ DBP_OptionCat::General,     DBP_OptionCat::General,     "Configuracoes gerais (teclas de atalho, menu iniciar, FPS fixo)." },
	#endif
	{ DBP_OptionCat::Input,       DBP_OptionCat::Input,       "Configuracoes de teclado, mouse e joystick." },
	{ DBP_OptionCat::Performance, DBP_OptionCat::Performance, "Ajuste o desempenho da CPU emulada." },
	{ DBP_OptionCat::Video,       DBP_OptionCat::Video,       "Configuracoes para a placa grafica emulada e proporcao de aspecto." },
	{ DBP_OptionCat::System,      DBP_OptionCat::System,      "Outras configuracoes de emulacao de hardware para RAM, CPU e SO." },
	{ DBP_OptionCat::Audio,       DBP_OptionCat::Audio,       "Configuracoes de MIDI, SoundBlaster e outras de audio." },
	{ NULL, NULL, NULL }
};

namespace DBP_Option
{
	enum Index
	{
		#ifdef DBP_STANDALONE
		// Interface
		_interface_hotkeymod,
		_interface_speedtoggle,
		_interface_fastrate,
		_interface_slowrate,
		_interface_systemhotkeys,
		_interface_middlemouse,
		_interface_lockmouse,
		#endif
		// General
		forcefps,
		#ifndef DBP_STANDALONE
		savestate,
		#endif
		strict_mode,
		conf,
		menu_time,
		menu_transparency,
		// Input
		map_osd,
		mouse_input,
		mouse_wheel,
		mouse_speed_factor,
		mouse_speed_factor_x,
		actionwheel_inputs,
		auto_mapping,
		keyboard_layout,
		joystick_analog_deadzone,
		joystick_timed,
		// Performance
		cycles,
		cycles_max,
		cycles_scale,
		cycle_limit,
		perfstats,
		// Video
		machine,
		cga,
		hercules,
		svga,
		svgamem,
		voodoo,
		voodoo_perf,
		voodoo_scale,
		voodoo_gamma,
		#ifdef DBP_STANDALONE
		interface_scaling,
		interface_crtfilter,
		interface_crtscanline,
		interface_crtblur,
		interface_crtmask,
		interface_crtcurvature,
		interface_crtcorner,
		#endif
		aspect_correction,
		overscan,
		// System
		memory_size,
		modem,
		cpu_type,
		cpu_core,
		bootos_ramdisk,
		bootos_dfreespace,
		bootos_forcenormal,
		// Audio
		#ifndef DBP_STANDALONE
		audiorate,
		#else
		_interface_audiolatency,
		#endif
		sblaster_conf,
		midi,
		sblaster_type,
		sblaster_adlib_mode,
		sblaster_adlib_emu,
		gus,
		tandysound,
		swapstereo,
		_OPTIONS_NULL_TERMINATOR, _OPTIONS_TOTAL,
	};

	const char* Get(Index idx, bool* was_modified = NULL);
	bool Apply(Section& section, const char* var_name, const char* new_value, bool disallow_in_game = false, bool need_restart = false, bool user_modified = false);
	bool GetAndApply(Section& section, const char* var_name, Index idx, bool disallow_in_game = false, bool need_restart = false);
	void SetDisplay(Index idx, bool visible);
	bool GetHidden(const retro_core_option_v2_definition& d);
};

static retro_core_option_v2_definition option_defs[DBP_Option::_OPTIONS_TOTAL] =
{
	// General
	#ifdef DBP_STANDALONE
	{
		"interface_hotkeymod",
		"Modificador de Tecla de Atalho", NULL,
		"Define quais teclas modificadoras precisam ser pressionadas para usar as teclas de atalho." "\n"
			"   F1  - Pausar/Continuar (F12 para avancar um quadro enquanto pausado)" "\n"
			"   F2  - Camera Lenta (alternar/enquanto pressionado)" "\n"
			"   F3  - Avanco Rapido (alternar/enquanto pressionado)" "\n"
			"   F5  - Salvamento Rapido" "\n"
			"   F7  - Tela Cheia/Janela" "\n"
			"   F9  - Carregamento Rapido" "\n"
			"   F11 - Travar Mouse" "\n"
			"   F12 - Alternar Menu na Tela", NULL,
		DBP_OptionCat::General,
		{
			{ "1", "CTRL" },
			{ "2", "ALT" },
			{ "4", "SHIFT" },
			{ "3", "CTRL+ALT" },
			{ "5", "CTRL+SHIFT" },
			{ "6", "ALT+SHIFT" },
			{ "7", "CTRL+ALT+SHIFT" },
			{ "8", "WIN" },
			{ "16", "MENU" },
			{ "0", "Nenhum" },
		},
		"1"
	},
	{
		"interface_speedtoggle",
		"Modo de Avanco Rapido/Camera Lenta", NULL,
		"Define se o avanco rapido e a camera lenta sao alternados ou mantidos.", NULL,
		DBP_OptionCat::General,
		{
			{ "toggle", "Alternar" },
			{ "hold", "Manter Pressionado" },
		},
		"toggle"
	},
	{
		"interface_fastrate",
		"Limite de Avanco Rapido", NULL,
		"Define o limite do avanco rapido.", NULL,
		DBP_OptionCat::General,
		{
			{ "1.1" , "110%" }, { "1.2" , "120%" }, { "1.3" , "130%" }, { "1.5" , "150%" }, { "1.75" , "175%" }, { "2" , "200%" }, { "2.5" , "250%" }, { "3" , "300%" },
			{ "4" , "400%" }, { "5" , "500%" }, { "6" , "600%" }, { "7" , "700%" }, { "8" , "800%" }, { "9" , "900%" }, { "10" , "1000%" }, { "0" , "O mais rapido possivel" }, 
		},
		"5"
	},
	{
		"interface_slowrate",
		"Velocidade da Camera Lenta", NULL,
		"Define a velocidade enquanto a camera lenta estiver ativa.", NULL,
		DBP_OptionCat::General,
		{
			{ "0.1", "10%" }, { "0.2", "20%" }, { "0.3", "30%" }, { "0.4", "40%" }, { "0.5", "50%" }, { "0.6", "60%" }, 
			{ "0.7", "70%" }, { "0.75", "75%" }, { "0.8", "80%" }, { "0.85", "85%" }, { "0.9", "90%" }, { "0.95", "95%" },
		},
		"0.3"
	},
	{
		"interface_systemhotkeys",
		"Sempre Ativar Teclas de Atalho do Sistema", NULL,
		"Define se ALT+F4 (Sair) e ALT+Enter (Tela Cheia) sao processados mesmo enquanto um jogo esta em execucao.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Desativado" },
			{ "true", "Ativado" },
		},
		"true"
	},
	{
		"interface_middlemouse",
		"Botao do Meio do Mouse Abre o Menu", NULL,
		"Se ativado, o botao do meio do mouse abrira/fechara o Menu na Tela.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Desativado" },
			{ "true", "Ativado" },
		},
		"false"
	},
	{
		"interface_lockmouse",
		"Estado Padrao do Travamento do Mouse", NULL,
		"Se ativado, o mouse ficara travado no inicio do programa.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Desativado" },
			{ "true", "Ativado" },
		},
		"false"
	},
	#endif
	{
		"dosbox_pure_force60fps", // legacy name
		"Forcar FPS de Saida", NULL, // Titulo atualizado
		"Ative para forcar a saida a uma taxa fixa. Tente 60 FPS se tiver problemas de 'screen tearing' ou sincronia de video." "\n"
		"A saida tera quadros pulados em taxas mais baixas e quadros duplicados em taxas mais altas.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Desativado" },
			{ "10",   "Ativado (10 FPS)" },
			{ "15",   "Ativado (15 FPS)" },
			{ "20",   "Ativado (20 FPS)" },
			{ "30",   "Ativado (30 FPS)" },
			{ "35",   "Ativado (35 FPS)" },
			{ "50",   "Ativado (50 FPS)" },
			{ "true", "Ativado (60 FPS)" },
			{ "70",   "Ativado (70 FPS)" },
			{ "90",   "Ativado (90 FPS)" },
			{ "120",  "Ativado (120 FPS)" },
			{ "144",  "Ativado (144 FPS)" },
			{ "240",  "Ativado (240 FPS)" },
			{ "360",  "Ativado (360 FPS)" },
		},
		"false"
	},
	#ifndef DBP_STANDALONE
	{
		"dosbox_pure_savestate",
		"Suporte para Salvar Estados", NULL,
		"Certifique-se de testa-lo em cada jogo antes de usa-lo. Jogos complexos do DOS do final da era podem ter problemas." "\n"
		"Lembre-se de que os estados salvos com configuracoes diferentes de video, CPU ou memoria nao podem ser carregados." "\n"
		"O suporte ao rebobinamento tem um alto custo de desempenho e precisa de pelo menos 40 MB de buffer de rebobinamento.", NULL,
		DBP_OptionCat::General,
		{
			{ "on",       "Ativar estados de salvamento" },
			{ "rewind",   "Ativar estados de salvamento com rebobinagem" },
			{ "disabled", "Desativado" },
		},
		"on"
	},
	#endif
	{
		"dosbox_pure_strict_mode",
		"Avancado > Usar Modo Estrito", NULL,
		"Desabilita a linha de comando, executando sistemas operacionais instalados e utilizando arquivos .BAT/.COM/.EXE/DOS.YML a partir do jogo salvo.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Desativado" },
			{ "true", "Ativado" },
		},
		"false"
	},
	{
		"dosbox_pure_conf",
		"Avancado > Carregamento de dosbox.conf", NULL,
		"O DOSBox Pure deve ser configurado via opcoes principais, mas opcionalmente suporta o carregamento de arquivos .conf legados.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Suporte a conf desabilitado (padrao)" },
			{ "inside", "Tentar 'dosbox.conf' no conteudo carregado (ZIP ou pasta)" },
			{ "outside", "Tentar '.conf' com o mesmo nome do conteudo carregado (ao lado do ZIP ou pasta)" },
		},
		"false"
	},
	{
		"dosbox_pure_menu_time",
		"Avancado > Menu Iniciar", NULL,
		"Definir o comportamento do menu Iniciar antes e depois de iniciar um jogo." "\n"
		"Voce tambem pode forcar a abertura mantendo pressionada a tecla Shift ou L2/R2 ao selecionar 'Reiniciar'.", NULL,
		DBP_OptionCat::General,
		{
			{ "99", "Mostrar no inicio, mostrar novamente apos a saida do jogo (padrao)" },
#ifndef STATIC_LINKING
			{ "5", "Mostrar no inicio, desligar o nucleo 5 segundos apos a saida do jogo iniciado automaticamente" },
			{ "3", "Mostrar no inicio, desligar o nucleo 3 segundos apos a saida do jogo iniciado automaticamente" },
			{ "0", "Mostrar no inicio, desligar o nucleo imediatamente apos a saida do jogo iniciado automaticamente" },
#endif
			{ "-1", "Sempre mostrar menu na inicializacao e apos a saida do jogo, ignorar a configuracao de inicio automatico" },
		},
		"99"
	},
	{
		"dosbox_pure_menu_transparency",
		"Avancado > Transparencia do Menu", NULL,
		"Defina o nivel de transparencia do Menu e do Teclado na Tela.", NULL,
		DBP_OptionCat::General,
		{
			{ "10", "10%" }, { "20", "20%" }, { "30", "30%" }, { "40", "40%" }, { "50", "50%" }, { "60", "60%" }, { "70", "70%" }, { "80", "80%" }, { "90", "90%" }, { "100", "100%" },
		},
		"70"
	},

	// Input
	{
		"dosbox_pure_on_screen_keyboard", // legacy name
		"Usar Botao L3 para Mostrar Menu", NULL, // <<< NOVA TRADUCAO
		"Sempre vincular o botao L3 do controle para mostrar o menu para trocar CDs/Disquetes e usar o Teclado na Tela.", NULL, // <<< NOVA TRADUCAO
		DBP_OptionCat::Input,
		{ { "true", "Ativado (Padrao para Menu)" }, { "keyboard", "Ativado (Padrao para Teclado na Tela)" }, { "false", "Desativado" } }, // <<< NOVA TRADUCAO
		"true"
	},
	{
		"dosbox_pure_mouse_input",
		"Modo de Entrada do Mouse", NULL,
		"Voce pode desativar o tratamento de entrada de um mouse ou uma tela sensivel ao toque (o mouse emulado atraves do joystick ainda funcionara)." "\n"
		"No modo de touchpad, use o arrastar para mover, toque para clicar, toque com dois dedos para clicar com o botao direito e pressione e segure para arrastar", NULL,
		DBP_OptionCat::Input,
#if defined(ANDROID) || defined(DBP_IOS) || defined(HAVE_LIBNX) || defined(_3DS) || defined(WIIU) || defined(VITA)
		{
			{ "pad", "Modo touchpad (padrao, ver descricao, melhor para telas sensiveis ao toque)" },
			{ "true", "Automatico (virtual ou direto)" },
			{ "virtual", "Movimento de mouse virtual" },
			{ "direct", "Mouse controlado diretamente (nao suportado por todos os jogos)" },
			{ "false", "Desativado (ignorar entradas do mouse)" },
		},
		"pad"
#else
		{
			{ "true", "Automatico (padrao)" },
			{ "virtual", "Movimento de mouse virtual" },
			{ "direct", "Mouse controlado diretamente (nao suportado por todos os jogos)" },
			{ "pad", "Modo touchpad (consulte a descricao, melhor para telas sensiveis ao toque)" },
			{ "false", "Desativado (ignorar entradas do mouse)" },
		},
		"true"
#endif
	},
	{
		"dosbox_pure_mouse_wheel",
		"Vincular Roda do Mouse a Tecla", NULL,
		"Vincule a roda do mouse para cima e para baixo a duas teclas do teclado para poder usa-la em jogos de DOS.", NULL,
		DBP_OptionCat::Input,
		{
			{ "67/68", "Colchete Esquerdo/Colchete Direito" },
			{ "72/71", "Virgula/Ponto" },
			{ "79/82", "Page-Up/Page-Down" },
			{ "78/81", "Home/End" },
			{ "80/82", "Delete/Page-Down" },
			{ "64/65", "Hifen/Igual" },
			{ "69/70", "Ponto e Virgula/Aspas" },
			{ "99/100", "Menos do Teclado Numerico/Mais do Teclado Numerico" },
			{ "97/98", "Dividir do Teclado Numerico/Multiplicar do Teclado Numerico" },
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
		DBP_OptionCat::Input,
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
		"Avancado > Sensibilidade Horizontal do Mouse", NULL,
		"Experimente com este valor se o mouse estiver muito rapido/lento ao se mover para a esquerda/direita.", NULL,
		DBP_OptionCat::Input,
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
		"Avancado > Entradas da Roda de Acao", NULL,
		"Define quais entradas controlam a roda de acao.", NULL,
		DBP_OptionCat::Input,
		{
			{ "14", "Alavanca Direita, D-Pad, Mouse (Padrao)" }, { "6",  "Alavanca Direita, D-Pad" }, { "10", "Alavanca Direita, Mouse" }, { "2",  "Alavanca Direita" },
			{ "15", "Ambas as Alavancas, D-Pad, Mouse" }, { "7",  "Ambas as Alavancas, D-Pad" }, { "11", "Ambas as Alavancas, Mouse" }, { "3",  "Ambas as Alavancas" },
			{ "13", "Alavanca Esquerda, D-Pad, Mouse" }, { "5",  "Alavanca Esquerda, D-Pad" }, { "9",  "Alavanca Esquerda, Mouse" }, { "1",  "Alavanca Esquerda" },
			{ "12", "D-Pad, Mouse" }, { "4",  "D-Pad" }, { "8",  "Mouse" },
		},
		"14"
	},
	{
		"dosbox_pure_auto_mapping",
		"Avancado > Mapeamentos Automaticos de Gamepad", NULL,
		"O DOSBox Pure pode aplicar automaticamente um esquema de mapeamento de controle de gamepad quando detecta um jogo." "\n"
		"Esses mapeamentos de botoes sao fornecidos pelo Projeto Keyb2Joypad (de Jemy Murphy e bigjim).", NULL,
		DBP_OptionCat::Input,
		{ { "true", "Ativado (padrao)" }, { "notify", "Ativar com notificacao na deteccao de jogo" }, { "false", "Desativado" } },
		"true"
	},
	{
		"dosbox_pure_keyboard_layout",
		"Avancado > Layout do Teclado", NULL,
		"Selecione o layout do teclado (nao afetara o Teclado na Tela).", NULL,
		DBP_OptionCat::Input,
		{
			{ "us",    "EUA (padrao)" },
			{ "uk",    "Reino Unido" },
			{ "be",    "Belgica" },
			{ "br",    "Brasil" },
			{ "hr",    "Croacia" },
			{ "cz243", "Republica Tcheca" },
			{ "dk",    "Dinamarca" },
			{ "su",    "Finlandia" },
			{ "fr",    "Franca" },
			{ "gr",    "Alemanha" },
			{ "gk",    "Grecia" },
			{ "hu",    "Hungria" },
			{ "is161", "Islandia" },
			{ "it",    "Italia" },
			{ "nl",    "Holanda" },
			{ "no",    "Noruega" },
			{ "pl",    "Polonia" },
			{ "po",    "Portugal" },
			{ "ru",    "Russia" },
			{ "sk",    "Eslovaquia" },
			{ "si",    "Eslovenia" },
			{ "sp",    "Espanha" },
			{ "sv",    "Suecia" },
			{ "sg",    "Suica (Alemao)" },
			{ "sf",    "Suica (Frances)" },
			{ "tr",    "Turquia" },
		},
		"br" // Mantido 'br' como padrao, conforme sua traducao anterior
	},
	{
		"dosbox_pure_joystick_analog_deadzone",
		"Avancado > Zona Morta Analogica do Joystick", NULL,
		"Defina a zona morta das alavancas analogicas do joystick. Pode ser usada para eliminar desvios causados por hardware de joystick mal calibrado.", NULL,
		DBP_OptionCat::Input,
		{
			{ "0",  "0%" }, { "5",  "5%" }, { "10", "10%" }, { "15", "15%" }, { "20", "20%" }, { "25", "25%" }, { "30", "30%" }, { "35", "35%" }, { "40", "40%" },
		},
		"15"
	},
	{
		"dosbox_pure_joystick_timed",
		"Avancado > Habilitar Intervalos Cronometrados do Joystick", NULL,
		"Habilitar intervalos cronometrados para os eixos do joystick. Experimente esta opcao se o seu joystick apresentar desvio." "\n\n", NULL, //end of Input > Advanced section
		DBP_OptionCat::Input,
		{ { "true", "Ativado (padrao)" }, { "false", "Desativado" } },
		"true"
	},

	// Performance
	{
		"dosbox_pure_cycles",
		"Desempenho Emulado", NULL,
		"O desempenho bruto que o DOSBox tentara emular." "\n\n", NULL, //end of Performance section
		DBP_OptionCat::Performance,
		{
			{ "auto",    "AUTO - O DOSBox tentara detectar as necessidades de desempenho (padrao)" },
			{ "max",     "MAX - Emular o maior numero de instrucoes possivel" },
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
		"Detalhado > Desempenho Emulado Maximo", NULL,
		"Com a velocidade de CPU dinamica (AUTO ou MAX acima), o nivel maximo de desempenho emulado.", NULL,
		DBP_OptionCat::Performance,
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
		"none"
	},
	{
		"dosbox_pure_cycles_scale",
		"Detalhado > Escala de Desempenho", NULL,
		"Ajuste fino do desempenho emulado para necessidades especificas.", NULL,
		DBP_OptionCat::Performance,
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
		"Quanto tempo por quadro deve ser usado pela emulacao ao emular o DOS o mais rapido possivel." "\n"
		"Diminua isso se o seu dispositivo esquentar enquanto usa este nucleo." "\n\n", NULL, //end of Performance > Detailed section
		DBP_OptionCat::Performance,
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
			//{ "1.01", "101%" }, { "1.02", "102%" }, { "1.1", "110%" }, { "1.2", "120%" } 
		},
		"1.0",
	},
	{
		"dosbox_pure_perfstats",
		"Avancado > Mostrar Estatisticas de Desempenho", NULL,
		"Ative para mostrar estatisticas sobre desempenho e taxa de quadros e verificar se a emulacao e executada em velocidade maxima.", NULL,
		DBP_OptionCat::Performance,
		{
			{ "none",     "Desativada" },
			{ "simple",   "Simples" },
			{ "detailed", "Informacoes detalhadas" },
		},
		"none"
	},

	// Video
	{
		"dosbox_pure_machine",
		"Chip Grafico Emulado (necessario reiniciar)", NULL,
		"O tipo de chip grafico que o DOSBox emulara.", NULL,
		DBP_OptionCat::Video,
		{
			{ "svga",     "SVGA (Super Video Graphics Array) (padrao)" },
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
		"Modo CGA", NULL,
		"A variacao de CGA que esta sendo emulada.", NULL,
		DBP_OptionCat::Video,
		{
			{ "early_auto", "Modelo antigo, modo composto automatico (padrao)" },
			{ "early_on",   "Modelo antigo, modo composto ligado" },
			{ "early_off",  "Modelo antigo, modo composto desligado" },
			{ "late_auto", "Modelo recente, modo composto automatico" },
			{ "late_on",   "Modelo recente, modo composto ligado" },
			{ "late_off",  "Modelo recente, modo composto desligado" },
		},
		"early_auto"
	},
	{
		"dosbox_pure_hercules",
		"Modo de Cor para Hercules", NULL,
		"O esquema de cores para a emulacao Hercules.", NULL,
		DBP_OptionCat::Video,
		{
			{ "white", "Preto e branco (padrao)" },
			{ "amber", "Preto e ambar" },
			{ "green", "Preto e verde" },
		},
		"white"
	},
	{
		"dosbox_pure_svga",
		"Modo SVGA (necessario reiniciar)", NULL,
		"A variacao SVGA que esta sendo emulada. Tente mudar isso se encontrar problemas graficos.", NULL,
		DBP_OptionCat::Video,
		{
			{ "svga_s3",       "S3 Trio64 (padrao)" },
			{ "vesa_nolfb",    "S3 Trio64 sem hack de buffer de linha (reduz cintilacao em alguns jogos)" },
			{ "vesa_oldvbe",   "S3 Trio64 VESA 1.3" },
			{ "svga_et3000",   "Tseng Labs ET3000" },
			{ "svga_et4000",   "Tseng Labs ET4000" },
			{ "svga_paradise", "Paradise PVGA1A" },
		},
		"svga_s3"
	},
	{
		"dosbox_pure_svgamem",
		"Memoria SVGA (necessaria reinicializacao)", NULL,
		"A quantidade de memoria disponivel para a placa SVGA emulada.", NULL,
		DBP_OptionCat::Video,
		{
			{ "0",  "512KB" },
			{ "1", "1MB" },
			{ "2", "2MB (padrao)" },
			{ "3", "3MB" },
			{ "4", "4MB" },
			{ "8", "8MB (nem sempre reconhecido)" },
		},
		"2"
	},
	{
		"dosbox_pure_voodoo",
		"Emulacao 3dfx Voodoo", NULL,
		"Habilita certos jogos com suporte para o acelerador 3D Voodoo." "\n"
		"Emulador 3dfx Voodoo Graphics SST-1/2 por Aaron Giles e a equipe do MAME (licenca: BSD-3-Clause)", NULL,
		DBP_OptionCat::Video,
		{
			{ "8mb", "Habilitado - 8MB de memoria (padrao)" },
			{ "12mb", "Habilitado - 12MB de memoria, Textura Dual" },
			{ "4mb", "Habilitado - 4MB de memoria, Somente Baixa Resolucao" },
			{ "off", "Desabilitado" },
		},
		"8mb",
	},
	{
		"dosbox_pure_voodoo_perf",
		"Desempenho 3dfx Voodoo", NULL,
		#ifndef DBP_STANDALONE
		"Opcoes para ajustar o comportamento da emulacao 3dfx Voodoo." "\n"
		"Mudar para OpenGL requer um reinicio." "\n"
		"Se o OpenGL estiver disponivel, a aceleracao 3D do lado do anfitriao e utilizada, o que pode tornar a renderizacao 3D muito mais rapida.\n"
		"Automatico usara OpenGL se for o driver de video ativo no frontend.", NULL,
		#else
		"Opcoes para ajustar o comportamento da emulacao 3dfx Voodoo.", NULL,
		#endif
		DBP_OptionCat::Video,
		{
			#ifndef DBP_STANDALONE
			{ "auto", "Automatico (padrao)" },
			{ "4", "Hardware OpenGL" },
			#else
			{ "auto", "Hardware OpenGL" },
			#endif
			{ "1", "Software Multi-Threaded" },
			{ "3", "Software Multi-Threaded, baixa qualidade" },
			{ "2", "Software Single-Threaded, baixa qualidade" },
			{ "0", "Software Single-Threaded" },
		},
		"auto",
	},
	{
		"dosbox_pure_voodoo_scale",
		"Escalonamento OpenGL 3dfx Voodoo", NULL,
		"Aumentar a resolucao nativa da imagem renderizada.", NULL,
		DBP_OptionCat::Video,
		{
			{ "1", "1x" }, { "2", "2x" }, { "3", "3x" }, { "4", "4x" }, { "5", "5x" }, { "6", "6x" }, { "7", "7x" }, { "8", "8x" },
		},
		"1",
	},
	{
		"dosbox_pure_voodoo_gamma",
		"Correcao de Gama 3dfx Voodoo", NULL,
		"Mudar o brilho da saida renderizada 3dfx.", NULL,
		DBP_OptionCat::Video,
		{
			{ "-10", "-10" }, { "-9", "-9" }, { "-8", "-8" }, { "-7", "-7" }, { "-6", "-6" }, { "-5", "-5" }, { "-4", "-4" }, { "-3", "-3" }, { "-2", "-2" }, { "-1", "-1" },
			{ "0", "Nenhum" },
			{ "1", "+1" }, { "2", "+2" }, { "3", "+3" }, { "4", "+4" }, { "5", "+5" }, { "6", "+6" }, { "7", "+7" }, { "8", "+8" }, { "9", "+9" }, { "10", "+10" },
			{ "999", "Desativar Correcao de Gama" },
		},
		"-2",
	},
	#ifdef DBP_STANDALONE
	{
		"interface_scaling",
		"Escalonamento", NULL,
		"Escolha como escalonar a exibicao do jogo para a resolucao da janela/tela cheia. O escalonamento por inteiro forcara todos os pixels a terem o mesmo tamanho, mas pode adicionar uma borda.", NULL,
		DBP_OptionCat::Video,
		{
			{ "default", "Escalonamento Nitido (padrao)" },
			{ "nearest", "Escalonamento Simples (vizinho mais proximo)" },
			{ "bilinear", "Escalonamento Bilinear" },
			{ "integer", "Escalonamento por Inteiro" },
		},
		"default"
	},
	{
		"interface_crtfilter",
		"Filtro CRT", NULL,
		"Habilita o efeito de filtro CRT na tela exibida (funciona melhor em telas de alta resolucao e sem escalonamento por inteiro).", NULL,
		DBP_OptionCat::Video,
		{
			{ "false", "Desativado" },
			{ "1", "Apenas Scanlines" },
			{ "2", "Fosforos estilo TV" },
			{ "3", "Fosforos de grade de abertura" },
			{ "4", "Fosforos estilo VGA esticado" },
			{ "5", "Fosforos estilo VGA" },
		},
		"false"
	},
	{
		"interface_crtscanline",
		"Filtro CRT - Intensidade da Scanline", NULL,
		NULL, NULL,
		DBP_OptionCat::Video,
		{
			{ "0", "Sem vaos de scanline" },
			{ "1", "Vaos mais fracos" },
			{ "2", "Vaos fracos" },
			{ "3", "Vaos normais" },
			{ "4", "Vaos fortes" },
			{ "5", "Vaos mais fortes" },
			{ "8", "Vaos fortissimos" },
		},
		"2"
	},
	{
		"interface_crtblur",
		"Filtro CRT - Desfoque/Nitidez", NULL,
		NULL, NULL,
		DBP_OptionCat::Video,
		{
			{ "0", "Embacado" },
			{ "1", "Suave" },
			{ "2", "Padrao" },
			{ "3", "Pixelado" },
			{ "4", "Mais nitido" },
			{ "7", "O mais nitido" },
		},
		"2"
	},
	{
		"interface_crtmask",
		"Filtro CRT - Forca da Mascara de Fosforo", NULL,
		NULL, NULL,
		DBP_OptionCat::Video,
		{
			{ "0", "Desativado" },
			{ "1", "Fraco" },
			{ "2", "Padrao" },
			{ "3", "Forte" },
			{ "4", "Muito Forte" },
		},
		"2"
	},
	{
		"interface_crtcurvature",
		"Filtro CRT - Curvatura", NULL,
		NULL, NULL,
		DBP_OptionCat::Video,
		{
			{ "0", "Desativado" },
			{ "1", "Fraca" },
			{ "2", "Padrao" },
			{ "3", "Forte" },
			{ "4", "Muito Forte" },
		},
		"2"
	},
	{
		"interface_crtcorner",
		"Filtro CRT - Canto Arredondado", NULL,
		NULL, NULL,
		DBP_OptionCat::Video,
		{
			{ "0", "Desativado" },
			{ "1", "Fraco" },
			{ "2", "Padrao" },
			{ "3", "Forte" },
			{ "4", "Muito Forte" },
		},
		"2"
	},
	#endif
	{
		"dosbox_pure_aspect_correction",
		"Correcao da Proporcao de Tela", NULL,
		"Ajusta a proporcao de tela para aproximar o que um monitor CRT exibiria (funciona melhor em telas de alta resolucao e sem escalonamento por inteiro).", NULL,
		DBP_OptionCat::Video,
		{
			{ "false", "Desativado (padrao)" },
			{ "true", "Ativado (escaneamento unico)" },
			{ "doublescan", "Ativado (escaneamento duplo quando aplicavel)" },
			{ "padded", "Ajustado para 4:3 (escaneamento unico)" },
			{ "padded-doublescan", "Ajustado para 4:3 (escaneamento duplo quando aplicavel)" },
			#ifdef DBP_STANDALONE
			{ "fill", "Esticar a tela para preencher a janela, ignorando a proporcao" }, // <<< NOVA TRADUCAO
			#endif
		},
		"false"
	},
	{
		"dosbox_pure_overscan",
		"Tamanho da Borda do Overscan", NULL,
		"Quando habilitado, mostra uma borda ao redor da tela. Alguns jogos usam a cor da borda para transmitir informacoes." "\n\n", NULL, // fim da secao de Video
		DBP_OptionCat::Video,
		{ { "0", "Desativado (padrao)" }, { "1", "Pequeno" }, { "2", "Medio" }, { "3", "Grande" } },
		"0"
	},

	// System
	{
		"dosbox_pure_memory_size",
		"Tamanho da Memoria (necessario reiniciar)", NULL,
		"A quantidade de memoria (alta) que a maquina emulada possui. Voce tambem pode desativar a memoria estendida (EMS/XMS)." "\n"
		"Nao e recomendado usar mais do que o padrao devido a incompatibilidade com certos jogos e aplicativos.", NULL,
		DBP_OptionCat::System,
		{
			{ "none", "Desativar memoria estendida (sem EMS/XMS)" },
			{ "4",  "4 MB" },
			{ "8",  "8 MB" },
			{ "16", "16 MB (padrao)" },
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
		"Tipo de modem emulado em COM1 para jogo em rede. Com o modem dial-up, um lado precisa discar qualquer numero para se conectar.", NULL,
		DBP_OptionCat::System,
		{
			{ "null", "Modem Nulo (Serial Direto)" },
			{ "dial", "Modem Dial-Up (Padrao Hayes)" },
		},
		"null"
	},
	{
		"dosbox_pure_cpu_type",
		"Tipo de CPU (necessario reiniciar)", NULL,
		"Tipo de CPU emulado. 'Auto' e a opcao mais rapida." "\n"
			"Jogos que requerem selecao especifica de tipo de CPU:" "\n"
			"386 (pre-busca): X-Men: Madness in The Murderworld, Terminator 1, Contra, Fifa International Soccer 1994" "\n"
			"486 (lento): Betrayal in Antara" "\n"
			"Pentium (lento): Fifa International Soccer 1994, jogos do Windows 95/Windows 3.x" "\n\n", NULL, //end of System section
		DBP_OptionCat::System,
		{
			{ "auto", "Automatico - Conjunto de recursos misto com maxima performance e compatibilidade" },
			{ "386", "386 - Conjunto de instrucoes 386 com acesso rapido a memoria" },
			{ "386_slow", "386 (lento) - Conjunto de instrucoes 386 com verificacoes de privilegio de memoria" },
			{ "386_prefetch", "386 (pre-busca) - Com emulacao de fila de pre-busca (apenas nos nucleos 'auto' e 'normal')" },
			{ "486_slow", "486 (lento) - Conjunto de instrucoes 486 com verificacoes de privilegio de memoria" },
			{ "pentium_slow", "Pentium (lento) - Conjunto de instrucoes 586 com verificacoes de privilegio de memoria" },
		},
		"auto"
	},
	{
		"dosbox_pure_cpu_core",
		"Avancado > Nucleo da CPU", NULL,
		"Metodo de emulacao (nucleo da CPU do DOSBox) usado.", NULL,
		DBP_OptionCat::System,
		{
			#if defined(C_DYNAMIC_X86)
			{ "auto", "Automatico - Jogos em modo real usam normal, jogos em modo protegido usam dinamico" },
			{ "dynamic", "Dinamico - Recompilacao dinamica (rapida, usando a implementacao dynamic_x86)" },
			#elif defined(C_DYNREC)
			{ "auto", "Automatico - Jogos em modo real usam normal, jogos em modo protegido usam dinamico" },
			{ "dynamic", "Dinamico - Recompilacao dinamica (rapida, usando a implementacao dynrec)" },
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
		"Avancado > Modificacoes no Disco do SO (necessario reiniciar)", NULL,
		"Ao executar um sistema operacional instalado, as modificacoes na unidade C: serao feitas na imagem de disco por padrao." "\n"
		"Definir para 'Descartar' permite que o conteudo seja fechado a qualquer momento sem preocupacoes com corrupcao do sistema de arquivos ou do registro." "\n"
		"Ao usar 'Salvar Diferenca por Conteudo', a imagem do disco nunca deve ser modificada novamente, caso contrario, as diferencas existentes se tornarao inutilizaveis.", NULL,
		DBP_OptionCat::System,
		{
			{ "false", "Manter (padrao)" },
			{ "true", "Descartar" },
			{ "diff", "Salvar Diferenca por Conteudo" },
		},
		"false"
	},
	{
		"dosbox_pure_bootos_dfreespace",
		"Avancado > Espaco Livre em D: no SO (necessario reiniciar)", NULL,
		"Controla a quantidade de espaco livre disponivel na unidade D: ao executar um sistema operacional instalado." "\n"
		"Se o tamanho total da unidade D: (dados + espaco livre) exceder 2 GB, nao podera ser usado nas versoes anteriores do Windows 95." "\n"
		"ATENCAO: Os arquivos de salvamento criados estao vinculados a essa configuracao, portanto, altera-la ocultara todas as alteracoes existentes na unidade D:.", NULL,
		DBP_OptionCat::System,
		{ { "1024", "1GB (padrao)" }, { "2048", "2GB" }, { "4096", "4GB" }, { "8192", "8GB" }, { "discard", "Descartar Alteracoes em D:" }, { "hide", "Desativar Disco Rigido D: (usar apenas CD-ROM)" } }, // <<< NOVA TRADUCAO
		"1024"
	},
	{
		"dosbox_pure_bootos_forcenormal",
		"Avancado > Forcar Nucleo Normal no SO", NULL,
		"O nucleo normal pode ser mais estavel ao executar um sistema operacional instalado." "\n"
		"Isso pode ser ligado e desligado para contornar travamentos." "\n\n", NULL, //end of System > Advanced section
		DBP_OptionCat::System,
		{ { "false", "Desativado (padrao)" }, { "true", "Ativado" } },
		"false"
	},

	// Audio
	#ifndef DBP_STANDALONE
	{
		"dosbox_pure_audiorate",
		"Taxa de Amostragem de Audio (necessario reiniciar)", NULL,
		"Isso deve corresponder a configuracao de taxa de saida de audio do frontend (Hz).", NULL,
		DBP_OptionCat::Audio,
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
	#else
	{
		"interface_audiolatency",
		"Latencia de Audio", NULL,
		"Se definido muito baixo, podem ocorrer falhas no audio. O valor e para processamento interno e a latencia percebida sera maior.", NULL,
		DBP_OptionCat::Audio,
		{
			{ "10", "10 ms" }, { "15", "15 ms" }, { "20", "20 ms" }, { "25", "25 ms" }, { "30", "30 ms" }, { "35", "35 ms" }, { "40", "40 ms" }, { "45", "45 ms" }, { "50", "50 ms" },
			{ "55", "55 ms" }, { "60", "60 ms" }, { "65", "65 ms" }, { "70", "70 ms" }, { "75", "75 ms" }, { "80", "80 ms" }, { "85", "85 ms" }, { "90", "90 ms" }, { "95", "95 ms" }, { "100", "100 ms" },
		},
		"25"
	},
	#endif
	{
		"dosbox_pure_sblaster_conf",
		"Configuracoes do SoundBlaster", NULL,
		"Defina o endereco, interrupcao, DMA de 8 bits baixos e DMA de 16 bits altos.", NULL,
		DBP_OptionCat::Audio,
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
		"Saida MIDI", NULL,
		"Selecione o arquivo SoundFont .SF2, arquivo .ROM ou interface usada para saida MIDI." "\n"
		#ifndef DBP_STANDALONE
		"Para adicionar SoundFonts ou arquivos ROM, copie-os para o diretorio 'system' do frontend." "\n"
		"Para usar o driver MIDI do frontend, certifique-se de que ele esteja configurado corretamente."
		#else
		"Para adicionar SoundFonts ou arquivos ROM, copie-os para o diretorio 'system' do DOSBox Pure." "\n" // <<< NOVA TRADUCAO
		#endif
		"\n\n", NULL, //end of Audio section
		DBP_OptionCat::Audio,
		{
			// dynamically filled in retro_init
		},
		"disabled"
	},
	{
		"dosbox_pure_sblaster_type",
		"Avancado > Tipo de SoundBlaster", NULL,
		"Tipo de placa SoundBlaster emulada.", NULL,
		DBP_OptionCat::Audio,
		{
			{ "sb16", "SoundBlaster 16 (padrao)" },
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
		"Avancado > Modo Adlib/FM do SoundBlaster", NULL,
		"O modo de sintese FM emulado pelo SoundBlaster. Todos os modos sao compativeis com o Adlib, exceto o CMS.", NULL,
		DBP_OptionCat::Audio,
		{
			{ "auto",     "Automatico (selecionar com base no tipo de SoundBlaster) (padrao)" },
			{ "cms",      "CMS (Creative Music System / GameBlaster)" },
			{ "opl2",     "OPL-2 (AdLib / OPL-2 / Yamaha 3812)" },
			{ "dualopl2", "Dual OPL-2 (Dual OPL-2 usado pelo SoundBlaster Pro 1.0 para som estereo)" },
			{ "opl3",     "OPL-3 (AdLib / OPL-3 / Yamaha YMF262)" },
			{ "opl3gold", "OPL-3 Gold (AdLib Gold / OPL-3 / Yamaha YMF262)" },
			{ "none",     "Desativado" },
		},
		"auto"
	},
	{
		"dosbox_pure_sblaster_adlib_emu",
		"Avancado > Provedor de Adlib SoundBlaster", NULL,
		"Provedor para a emulacao do Adlib. O padrao possui boa qualidade e baixos requisitos de desempenho.", NULL,
		DBP_OptionCat::Audio,
		{
			{ "default", "Padrao" },
			{ "nuked", "Alta qualidade Nuked OPL3" },
		},
		"default"
	},
	{
		"dosbox_pure_gus",
		"Avancado > Habilitar Emulacao do Gravis Ultrasound (necessario reiniciar)", NULL,
		"Habilitar emulacao do Gravis Ultrasound. As configuracoes estao fixadas em porta 0x240, IRQ 5, DMA 3." "\n"
		"Se a variavel ULTRADIR precisar ser diferente do padrao 'C:\\ULTRASND' voce precisara inserir 'SET ULTRADIR=...' na linha de comando ou em um arquivo em lote.", NULL,
		DBP_OptionCat::Audio,
		{ { "false", "Desativado (padrao)" }, { "true", "Ativado" } },
		"false"
	},
	{
		"dosbox_pure_tandysound",
		"Avancado > Habilitar Dispositivo de Som Tandy (reinicio necessario)", NULL,
		"Habilita a emulacao do Dispositivo de Som Tandy mesmo quando executando sem a emulacao do Adaptador Grafico Tandy.", NULL,
		DBP_OptionCat::Audio,
		{ { "auto", "Desativado (padrao)" }, { "on", "Ativado" } },
		"auto"
	},
	{
		"dosbox_pure_swapstereo",
		"Avancado > Trocar Canais Estereo", NULL,
		"Trocar o canal de audio esquerdo e direito." "\n\n", NULL, //end of Audio > Advanced section
		DBP_OptionCat::Audio,
		{ { "false", "Desativado (padrao)" }, { "true", "Ativado" } },
		"false"
	},

	{ NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL }
};