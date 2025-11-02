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
	static const char* Video       = "V¡deo";
	static const char* System      = "Sistema";
	static const char* Audio       = "†udio";
};

static retro_core_option_v2_category option_cats[] =
{
	#ifndef DBP_STANDALONE
	{ DBP_OptionCat::General,     DBP_OptionCat::General,     "Configura‡”es gerais (estados de salvamento, menu iniciar, FPS fixo)." },
	#else
	{ DBP_OptionCat::General,     DBP_OptionCat::General,     "Configura‡”es gerais (teclas de atalho, menu iniciar, FPS fixo)." },
	#endif
	{ DBP_OptionCat::Input,       DBP_OptionCat::Input,       "Configura‡”es de teclado, mouse e joystick." },
	{ DBP_OptionCat::Performance, DBP_OptionCat::Performance, "Ajuste o desempenho da CPU emulada." },
	{ DBP_OptionCat::Video,       DBP_OptionCat::Video,       "Configura‡”es para a placa gr fica emulada e propor‡„o de aspecto." },
	{ DBP_OptionCat::System,      DBP_OptionCat::System,      "Outras configura‡”es de emula‡„o de hardware para RAM, CPU e SO." },
	{ DBP_OptionCat::Audio,       DBP_OptionCat::Audio,       "Configura‡”es de MIDI, SoundBlaster e outras de  udio." },
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
			"   F1  - Pausar/Continuar (F12 para avan‡ar um quadro enquanto pausado)" "\n"
			"   F2  - Cƒmera Lenta (alternar/enquanto pressionado)" "\n"
			"   F3  - Avan‡o R pido (alternar/enquanto pressionado)" "\n"
			"   F5  - Salvamento R pido" "\n"
			"   F7  - Tela Cheia/Janela" "\n"
			"   F9  - Carregamento R pido" "\n"
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
		"Modo de Avan‡o R pido/Cƒmera Lenta", NULL,
		"Define se o avan‡o r pido e a cƒmera lenta s„o alternados ou mantidos.", NULL,
		DBP_OptionCat::General,
		{
			{ "toggle", "Alternar" },
			{ "hold", "Manter Pressionado" },
		},
		"toggle"
	},
	{
		"interface_fastrate",
		"Limite de Avan‡o R pido", NULL,
		"Define o limite do avan‡o r pido.", NULL,
		DBP_OptionCat::General,
		{
			{ "1.1" , "110%" }, { "1.2" , "120%" }, { "1.3" , "130%" }, { "1.5" , "150%" }, { "1.75" , "175%" }, { "2" , "200%" }, { "2.5" , "250%" }, { "3" , "300%" },
			{ "4" , "400%" }, { "5" , "500%" }, { "6" , "600%" }, { "7" , "700%" }, { "8" , "800%" }, { "9" , "900%" }, { "10" , "1000%" }, { "0" , "O mais r pido poss¡vel" }, 
		},
		"5"
	},
	{
		"interface_slowrate",
		"Velocidade da Cƒmera Lenta", NULL,
		"Define a velocidade enquanto a cƒmera lenta estiver ativa.", NULL,
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
		"Define se ALT+F4 (Sair) e ALT+Enter (Tela Cheia) s„o processados mesmo enquanto um jogo est  em execu‡„o.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Desativado" },
			{ "true", "Ativado" },
		},
		"true"
	},
	{
		"interface_middlemouse",
		"Bot„o do Meio do Mouse Abre o Menu", NULL,
		"Se ativado, o bot„o do meio do mouse abrir /fechar  o Menu na Tela.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Desativado" },
			{ "true", "Ativado" },
		},
		"false"
	},
	{
		"interface_lockmouse",
		"Estado Padr„o do Travamento do Mouse", NULL,
		"Se ativado, o mouse ficar  travado no in¡cio do programa.", NULL,
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
		"For‡ar FPS de Sa¡da", NULL, // T¡tulo atualizado
		"Ative para for‡ar a sa¡da a uma taxa fixa. Tente 60 FPS se tiver problemas de 'screen tearing' ou sincronia de v¡deo." "\n"
		"A sa¡da ter  quadros pulados em taxas mais baixas e quadros duplicados em taxas mais altas.", NULL,
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
		"Certifique-se de test -lo em cada jogo antes de us -lo. Jogos complexos do DOS do final da era podem ter problemas." "\n"
		"Lembre-se de que os estados salvos com configura‡”es diferentes de v¡deo, CPU ou mem¢ria n„o podem ser carregados." "\n"
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
		"Avan‡ado > Usar Modo Estrito", NULL,
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
		"Avan‡ado > Carregamento de dosbox.conf", NULL,
		"O DOSBox Pure deve ser configurado via op‡”es principais, mas opcionalmente suporta o carregamento de arquivos .conf legados.", NULL,
		DBP_OptionCat::General,
		{
			{ "false", "Suporte a conf desabilitado (padr„o)" },
			{ "inside", "Tentar 'dosbox.conf' no conte£do carregado (ZIP ou pasta)" },
			{ "outside", "Tentar '.conf' com o mesmo nome do conte£do carregado (ao lado do ZIP ou pasta)" },
		},
		"false"
	},
	{
		"dosbox_pure_menu_time",
		"Avan‡ado > Menu Iniciar", NULL,
		"Definir o comportamento do menu Iniciar antes e depois de iniciar um jogo." "\n"
		"Vocˆ tamb‚m pode for‡ar a abertura mantendo pressionada a tecla Shift ou L2/R2 ao selecionar 'Reiniciar'.", NULL,
		DBP_OptionCat::General,
		{
			{ "99", "Mostrar no in¡cio, mostrar novamente ap¢s a sa¡da do jogo (padr„o)" },
#ifndef STATIC_LINKING
			{ "5", "Mostrar no in¡cio, desligar o n£cleo 5 segundos ap¢s a sa¡da do jogo iniciado automaticamente" },
			{ "3", "Mostrar no in¡cio, desligar o n£cleo 3 segundos ap¢s a sa¡da do jogo iniciado automaticamente" },
			{ "0", "Mostrar no in¡cio, desligar o n£cleo imediatamente ap¢s a sa¡da do jogo iniciado automaticamente" },
#endif
			{ "-1", "Sempre mostrar menu na inicializa‡„o e ap¢s a sa¡da do jogo, ignorar a configura‡„o de in¡cio autom tico" },
		},
		"99"
	},
	{
		"dosbox_pure_menu_transparency",
		"Avan‡ado > Transparˆncia do Menu", NULL,
		"Defina o n¡vel de transparˆncia do Menu e do Teclado na Tela.", NULL,
		DBP_OptionCat::General,
		{
			{ "10", "10%" }, { "20", "20%" }, { "30", "30%" }, { "40", "40%" }, { "50", "50%" }, { "60", "60%" }, { "70", "70%" }, { "80", "80%" }, { "90", "90%" }, { "100", "100%" },
		},
		"70"
	},

	// Input
	{
		"dosbox_pure_on_screen_keyboard", // legacy name
		"Usar Bot„o L3 para Mostrar Menu", NULL, // <<< NOVA TRADU€ŽO
		"Sempre vincular o bot„o L3 do controle para mostrar o menu para trocar CDs/Disquetes e usar o Teclado na Tela.", NULL, // <<< NOVA TRADU€ŽO
		DBP_OptionCat::Input,
		{ { "true", "Ativado (Padr„o para Menu)" }, { "keyboard", "Ativado (Padr„o para Teclado na Tela)" }, { "false", "Desativado" } }, // <<< NOVA TRADU€ŽO
		"true"
	},
	{
		"dosbox_pure_mouse_input",
		"Modo de Entrada do Mouse", NULL,
		"Vocˆ pode desativar o tratamento de entrada de um mouse ou uma tela sens¡vel ao toque (o mouse emulado atrav‚s do joystick ainda funcionar )." "\n"
		"No modo de touchpad, use o arrastar para mover, toque para clicar, toque com dois dedos para clicar com o bot„o direito e pressione e segure para arrastar", NULL,
		DBP_OptionCat::Input,
#if defined(ANDROID) || defined(DBP_IOS) || defined(HAVE_LIBNX) || defined(_3DS) || defined(WIIU) || defined(VITA)
		{
			{ "pad", "Modo touchpad (padr„o, ver descri‡„o, melhor para telas sens¡veis ao toque)" },
			{ "true", "Autom tico (virtual ou direto)" },
			{ "virtual", "Movimento de mouse virtual" },
			{ "direct", "Mouse controlado diretamente (n„o suportado por todos os jogos)" },
			{ "false", "Desativado (ignorar entradas do mouse)" },
		},
		"pad"
#else
		{
			{ "true", "Autom tico (padr„o)" },
			{ "virtual", "Movimento de mouse virtual" },
			{ "direct", "Mouse controlado diretamente (n„o suportado por todos os jogos)" },
			{ "pad", "Modo touchpad (consulte a descri‡„o, melhor para telas sens¡veis ao toque)" },
			{ "false", "Desativado (ignorar entradas do mouse)" },
		},
		"true"
#endif
	},
	{
		"dosbox_pure_mouse_wheel",
		"Vincular Roda do Mouse a Tecla", NULL,
		"Vincule a roda do mouse para cima e para baixo a duas teclas do teclado para poder us -la em jogos de DOS.", NULL,
		DBP_OptionCat::Input,
		{
			{ "67/68", "Colchete Esquerdo/Colchete Direito" },
			{ "72/71", "V¡rgula/Ponto" },
			{ "79/82", "Page-Up/Page-Down" },
			{ "78/81", "Home/End" },
			{ "80/82", "Delete/Page-Down" },
			{ "64/65", "H¡fen/Igual" },
			{ "69/70", "Ponto e V¡rgula/Aspas" },
			{ "99/100", "Menos do Teclado Num‚rico/Mais do Teclado Num‚rico" },
			{ "97/98", "Dividir do Teclado Num‚rico/Multiplicar do Teclado Num‚rico" },
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
		"Avan‡ado > Sensibilidade Horizontal do Mouse", NULL,
		"Experimente com este valor se o mouse estiver muito r pido/lento ao se mover para a esquerda/direita.", NULL,
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
		"Avan‡ado > Entradas da Roda de A‡„o", NULL,
		"Define quais entradas controlam a roda de a‡„o.", NULL,
		DBP_OptionCat::Input,
		{
			{ "14", "Alavanca Direita, D-Pad, Mouse (Padr„o)" }, { "6",  "Alavanca Direita, D-Pad" }, { "10", "Alavanca Direita, Mouse" }, { "2",  "Alavanca Direita" },
			{ "15", "Ambas as Alavancas, D-Pad, Mouse" }, { "7",  "Ambas as Alavancas, D-Pad" }, { "11", "Ambas as Alavancas, Mouse" }, { "3",  "Ambas as Alavancas" },
			{ "13", "Alavanca Esquerda, D-Pad, Mouse" }, { "5",  "Alavanca Esquerda, D-Pad" }, { "9",  "Alavanca Esquerda, Mouse" }, { "1",  "Alavanca Esquerda" },
			{ "12", "D-Pad, Mouse" }, { "4",  "D-Pad" }, { "8",  "Mouse" },
		},
		"14"
	},
	{
		"dosbox_pure_auto_mapping",
		"Avan‡ado > Mapeamentos Autom ticos de Gamepad", NULL,
		"O DOSBox Pure pode aplicar automaticamente um esquema de mapeamento de controle de gamepad quando detecta um jogo." "\n"
		"Esses mapeamentos de bot”es s„o fornecidos pelo Projeto Keyb2Joypad (de Jemy Murphy e bigjim).", NULL,
		DBP_OptionCat::Input,
		{ { "true", "Ativado (padr„o)" }, { "notify", "Ativar com notifica‡„o na detec‡„o de jogo" }, { "false", "Desativado" } },
		"true"
	},
	{
		"dosbox_pure_keyboard_layout",
		"Avan‡ado > Layout do Teclado", NULL,
		"Selecione o layout do teclado (n„o afetar  o Teclado na Tela).", NULL,
		DBP_OptionCat::Input,
		{
			{ "us",    "EUA (padr„o)" },
			{ "uk",    "Reino Unido" },
			{ "be",    "B‚lgica" },
			{ "br",    "Brasil" },
			{ "hr",    "Cro cia" },
			{ "cz243", "Rep£blica Tcheca" },
			{ "dk",    "Dinamarca" },
			{ "su",    "Finlƒndia" },
			{ "fr",    "Fran‡a" },
			{ "gr",    "Alemanha" },
			{ "gk",    "Gr‚cia" },
			{ "hu",    "Hungria" },
			{ "is161", "Islƒndia" },
			{ "it",    "It lia" },
			{ "nl",    "Holanda" },
			{ "no",    "Noruega" },
			{ "pl",    "Pol“nia" },
			{ "po",    "Portugal" },
			{ "ru",    "R£ssia" },
			{ "sk",    "Eslov quia" },
			{ "si",    "Eslovˆnia" },
			{ "sp",    "Espanha" },
			{ "sv",    "Su‚cia" },
			{ "sg",    "Su¡‡a (Alem„o)" },
			{ "sf",    "Su¡‡a (Francˆs)" },
			{ "tr",    "Turquia" },
		},
		"br" // Mantido 'br' como padr„o, conforme sua tradu‡„o anterior
	},
	{
		"dosbox_pure_joystick_analog_deadzone",
		"Avan‡ado > Zona Morta Anal¢gica do Joystick", NULL,
		"Defina a zona morta das alavancas anal¢gicas do joystick. Pode ser usada para eliminar desvios causados por hardware de joystick mal calibrado.", NULL,
		DBP_OptionCat::Input,
		{
			{ "0",  "0%" }, { "5",  "5%" }, { "10", "10%" }, { "15", "15%" }, { "20", "20%" }, { "25", "25%" }, { "30", "30%" }, { "35", "35%" }, { "40", "40%" },
		},
		"15"
	},
	{
		"dosbox_pure_joystick_timed",
		"Avan‡ado > Habilitar Intervalos Cronometrados do Joystick", NULL,
		"Habilitar intervalos cronometrados para os eixos do joystick. Experimente esta op‡„o se o seu joystick apresentar desvio." "\n\n", NULL, //end of Input > Advanced section
		DBP_OptionCat::Input,
		{ { "true", "Ativado (padr„o)" }, { "false", "Desativado" } },
		"true"
	},

	// Performance
	{
		"dosbox_pure_cycles",
		"Desempenho Emulado", NULL,
		"O desempenho bruto que o DOSBox tentar  emular." "\n\n", NULL, //end of Performance section
		DBP_OptionCat::Performance,
		{
			{ "auto",    "AUTO - O DOSBox tentar  detectar as necessidades de desempenho (padr„o)" },
			{ "max",     "MAX - Emular o maior n£mero de instru‡”es poss¡vel" },
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
		"Detalhado > Desempenho Emulado M ximo", NULL,
		"Com a velocidade de CPU dinƒmica (AUTO ou MAX acima), o n¡vel m ximo de desempenho emulado.", NULL,
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
		"Ajuste fino do desempenho emulado para necessidades espec¡ficas.", NULL,
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
		"Quanto tempo por quadro deve ser usado pela emula‡„o ao emular o DOS o mais r pido poss¡vel." "\n"
		"Diminua isso se o seu dispositivo esquentar enquanto usa este n£cleo." "\n\n", NULL, //end of Performance > Detailed section
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
		"Avan‡ado > Mostrar Estat¡sticas de Desempenho", NULL,
		"Ative para mostrar estat¡sticas sobre desempenho e taxa de quadros e verificar se a emula‡„o ‚ executada em velocidade m xima.", NULL,
		DBP_OptionCat::Performance,
		{
			{ "none",     "Desativada" },
			{ "simple",   "Simples" },
			{ "detailed", "Informa‡”es detalhadas" },
		},
		"none"
	},

	// Video
	{
		"dosbox_pure_machine",
		"Chip Gr fico Emulado (necess rio reiniciar)", NULL,
		"O tipo de chip gr fico que o DOSBox emular .", NULL,
		DBP_OptionCat::Video,
		{
			{ "svga",     "SVGA (Super Video Graphics Array) (padr„o)" },
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
		"A varia‡„o de CGA que est  sendo emulada.", NULL,
		DBP_OptionCat::Video,
		{
			{ "early_auto", "Modelo antigo, modo composto autom tico (padr„o)" },
			{ "early_on",   "Modelo antigo, modo composto ligado" },
			{ "early_off",  "Modelo antigo, modo composto desligado" },
			{ "late_auto", "Modelo recente, modo composto autom tico" },
			{ "late_on",   "Modelo recente, modo composto ligado" },
			{ "late_off",  "Modelo recente, modo composto desligado" },
		},
		"early_auto"
	},
	{
		"dosbox_pure_hercules",
		"Modo de Cor para Hercules", NULL,
		"O esquema de cores para a emula‡„o Hercules.", NULL,
		DBP_OptionCat::Video,
		{
			{ "white", "Preto e branco (padr„o)" },
			{ "amber", "Preto e ƒmbar" },
			{ "green", "Preto e verde" },
		},
		"white"
	},
	{
		"dosbox_pure_svga",
		"Modo SVGA (necess rio reiniciar)", NULL,
		"A varia‡„o SVGA que est  sendo emulada. Tente mudar isso se encontrar problemas gr ficos.", NULL,
		DBP_OptionCat::Video,
		{
			{ "svga_s3",       "S3 Trio64 (padr„o)" },
			{ "vesa_nolfb",    "S3 Trio64 sem hack de buffer de linha (reduz cintila‡„o em alguns jogos)" },
			{ "vesa_oldvbe",   "S3 Trio64 VESA 1.3" },
			{ "svga_et3000",   "Tseng Labs ET3000" },
			{ "svga_et4000",   "Tseng Labs ET4000" },
			{ "svga_paradise", "Paradise PVGA1A" },
		},
		"svga_s3"
	},
	{
		"dosbox_pure_svgamem",
		"Mem¢ria SVGA (necess ria reinicializa‡„o)", NULL,
		"A quantidade de mem¢ria dispon¡vel para a placa SVGA emulada.", NULL,
		DBP_OptionCat::Video,
		{
			{ "0",  "512KB" },
			{ "1", "1MB" },
			{ "2", "2MB (padr„o)" },
			{ "3", "3MB" },
			{ "4", "4MB" },
			{ "8", "8MB (nem sempre reconhecido)" },
		},
		"2"
	},
	{
		"dosbox_pure_voodoo",
		"Emula‡„o 3dfx Voodoo", NULL,
		"Habilita certos jogos com suporte para o acelerador 3D Voodoo." "\n"
		"Emulador 3dfx Voodoo Graphics SST-1/2 por Aaron Giles e a equipe do MAME (licen‡a: BSD-3-Clause)", NULL,
		DBP_OptionCat::Video,
		{
			{ "8mb", "Habilitado - 8MB de mem¢ria (padr„o)" },
			{ "12mb", "Habilitado - 12MB de mem¢ria, Textura Dual" },
			{ "4mb", "Habilitado - 4MB de mem¢ria, Somente Baixa Resolu‡„o" },
			{ "off", "Desabilitado" },
		},
		"8mb",
	},
	{
		"dosbox_pure_voodoo_perf",
		"Desempenho 3dfx Voodoo", NULL,
		#ifndef DBP_STANDALONE
		"Op‡”es para ajustar o comportamento da emula‡„o 3dfx Voodoo." "\n"
		"Mudar para OpenGL requer um rein¡cio." "\n"
		"Se o OpenGL estiver dispon¡vel, a acelera‡„o 3D do lado do anfitri„o ‚ utilizada, o que pode tornar a renderiza‡„o 3D muito mais r pida.\n"
		"Autom tico usar  OpenGL se for o driver de v¡deo ativo no frontend.", NULL,
		#else
		"Op‡”es para ajustar o comportamento da emula‡„o 3dfx Voodoo.", NULL,
		#endif
		DBP_OptionCat::Video,
		{
			#ifndef DBP_STANDALONE
			{ "auto", "Autom tico (padr„o)" },
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
		"Aumentar a resolu‡„o nativa da imagem renderizada.", NULL,
		DBP_OptionCat::Video,
		{
			{ "1", "1x" }, { "2", "2x" }, { "3", "3x" }, { "4", "4x" }, { "5", "5x" }, { "6", "6x" }, { "7", "7x" }, { "8", "8x" },
		},
		"1",
	},
	{
		"dosbox_pure_voodoo_gamma",
		"Corre‡„o de Gama 3dfx Voodoo", NULL,
		"Mudar o brilho da sa¡da renderizada 3dfx.", NULL,
		DBP_OptionCat::Video,
		{
			{ "-10", "-10" }, { "-9", "-9" }, { "-8", "-8" }, { "-7", "-7" }, { "-6", "-6" }, { "-5", "-5" }, { "-4", "-4" }, { "-3", "-3" }, { "-2", "-2" }, { "-1", "-1" },
			{ "0", "Nenhum" },
			{ "1", "+1" }, { "2", "+2" }, { "3", "+3" }, { "4", "+4" }, { "5", "+5" }, { "6", "+6" }, { "7", "+7" }, { "8", "+8" }, { "9", "+9" }, { "10", "+10" },
			{ "999", "Desativar Corre‡„o de Gama" },
		},
		"-2",
	},
	#ifdef DBP_STANDALONE
	{
		"interface_scaling",
		"Escalonamento", NULL,
		"Escolha como escalonar a exibi‡„o do jogo para a resolu‡„o da janela/tela cheia. O escalonamento por inteiro for‡ar  todos os pixels a terem o mesmo tamanho, mas pode adicionar uma borda.", NULL,
		DBP_OptionCat::Video,
		{
			{ "default", "Escalonamento N¡tido (padr„o)" },
			{ "nearest", "Escalonamento Simples (vizinho mais pr¢ximo)" },
			{ "bilinear", "Escalonamento Bilinear" },
			{ "integer", "Escalonamento por Inteiro" },
		},
		"default"
	},
	{
		"interface_crtfilter",
		"Filtro CRT", NULL,
		"Habilita o efeito de filtro CRT na tela exibida (funciona melhor em telas de alta resolu‡„o e sem escalonamento por inteiro).", NULL,
		DBP_OptionCat::Video,
		{
			{ "false", "Desativado" },
			{ "1", "Apenas Scanlines" },
			{ "2", "F¢sforos estilo TV" },
			{ "3", "F¢sforos de grade de abertura" },
			{ "4", "F¢sforos estilo VGA esticado" },
			{ "5", "F¢sforos estilo VGA" },
		},
		"false"
	},
	{
		"interface_crtscanline",
		"Filtro CRT - Intensidade da Scanline", NULL,
		NULL, NULL,
		DBP_OptionCat::Video,
		{
			{ "0", "Sem v„os de scanline" },
			{ "1", "V„os mais fracos" },
			{ "2", "V„os fracos" },
			{ "3", "V„os normais" },
			{ "4", "V„os fortes" },
			{ "5", "V„os mais fortes" },
			{ "8", "V„os fort¡ssimos" },
		},
		"2"
	},
	{
		"interface_crtblur",
		"Filtro CRT - Desfoque/Nitidez", NULL,
		NULL, NULL,
		DBP_OptionCat::Video,
		{
			{ "0", "Emba‡ado" },
			{ "1", "Suave" },
			{ "2", "Padr„o" },
			{ "3", "Pixelado" },
			{ "4", "Mais n¡tido" },
			{ "7", "O mais n¡tido" },
		},
		"2"
	},
	{
		"interface_crtmask",
		"Filtro CRT - For‡a da M scara de F¢sforo", NULL,
		NULL, NULL,
		DBP_OptionCat::Video,
		{
			{ "0", "Desativado" },
			{ "1", "Fraco" },
			{ "2", "Padr„o" },
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
			{ "2", "Padr„o" },
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
			{ "2", "Padr„o" },
			{ "3", "Forte" },
			{ "4", "Muito Forte" },
		},
		"2"
	},
	#endif
	{
		"dosbox_pure_aspect_correction",
		"Corre‡„o da Propor‡„o de Tela", NULL,
		"Ajusta a propor‡„o de tela para aproximar o que um monitor CRT exibiria (funciona melhor em telas de alta resolu‡„o e sem escalonamento por inteiro).", NULL,
		DBP_OptionCat::Video,
		{
			{ "false", "Desativado (padr„o)" },
			{ "true", "Ativado (escaneamento £nico)" },
			{ "doublescan", "Ativado (escaneamento duplo quando aplic vel)" },
			{ "padded", "Ajustado para 4:3 (escaneamento £nico)" },
			{ "padded-doublescan", "Ajustado para 4:3 (escaneamento duplo quando aplic vel)" },
			#ifdef DBP_STANDALONE
			{ "fill", "Esticar a tela para preencher a janela, ignorando a propor‡„o" }, // <<< NOVA TRADU€ŽO
			#endif
		},
		"false"
	},
	{
		"dosbox_pure_overscan",
		"Tamanho da Borda do Overscan", NULL,
		"Quando habilitado, mostra uma borda ao redor da tela. Alguns jogos usam a cor da borda para transmitir informa‡”es." "\n\n", NULL, // fim da se‡„o de V¡deo
		DBP_OptionCat::Video,
		{ { "0", "Desativado (padr„o)" }, { "1", "Pequeno" }, { "2", "M‚dio" }, { "3", "Grande" } },
		"0"
	},

	// System
	{
		"dosbox_pure_memory_size",
		"Tamanho da Mem¢ria (necess rio reiniciar)", NULL,
		"A quantidade de mem¢ria (alta) que a m quina emulada possui. Vocˆ tamb‚m pode desativar a mem¢ria estendida (EMS/XMS)." "\n"
		"N„o ‚ recomendado usar mais do que o padr„o devido … incompatibilidade com certos jogos e aplicativos.", NULL,
		DBP_OptionCat::System,
		{
			{ "none", "Desativar mem¢ria estendida (sem EMS/XMS)" },
			{ "4",  "4 MB" },
			{ "8",  "8 MB" },
			{ "16", "16 MB (padr„o)" },
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
		"Tipo de modem emulado em COM1 para jogo em rede. Com o modem dial-up, um lado precisa discar qualquer n£mero para se conectar.", NULL,
		DBP_OptionCat::System,
		{
			{ "null", "Modem Nulo (Serial Direto)" },
			{ "dial", "Modem Dial-Up (Padr„o Hayes)" },
		},
		"null"
	},
	{
		"dosbox_pure_cpu_type",
		"Tipo de CPU (necess rio reiniciar)", NULL,
		"Tipo de CPU emulado. 'Auto' ‚ a op‡„o mais r pida." "\n"
			"Jogos que requerem sele‡„o espec¡fica de tipo de CPU:" "\n"
			"386 (pr‚-busca): X-Men: Madness in The Murderworld, Terminator 1, Contra, Fifa International Soccer 1994" "\n"
			"486 (lento): Betrayal in Antara" "\n"
			"Pentium (lento): Fifa International Soccer 1994, jogos do Windows 95/Windows 3.x" "\n\n", NULL, //end of System section
		DBP_OptionCat::System,
		{
			{ "auto", "Autom tico - Conjunto de recursos misto com m xima performance e compatibilidade" },
			{ "386", "386 - Conjunto de instru‡”es 386 com acesso r pido … mem¢ria" },
			{ "386_slow", "386 (lento) - Conjunto de instru‡”es 386 com verifica‡”es de privil‚gio de mem¢ria" },
			{ "386_prefetch", "386 (pr‚-busca) - Com emula‡„o de fila de pr‚-busca (apenas nos n£cleos 'auto' e 'normal')" },
			{ "486_slow", "486 (lento) - Conjunto de instru‡”es 486 com verifica‡”es de privil‚gio de mem¢ria" },
			{ "pentium_slow", "Pentium (lento) - Conjunto de instru‡”es 586 com verifica‡”es de privil‚gio de mem¢ria" },
		},
		"auto"
	},
	{
		"dosbox_pure_cpu_core",
		"Avan‡ado > N£cleo da CPU", NULL,
		"M‚todo de emula‡„o (n£cleo da CPU do DOSBox) usado.", NULL,
		DBP_OptionCat::System,
		{
			#if defined(C_DYNAMIC_X86)
			{ "auto", "Autom tico - Jogos em modo real usam normal, jogos em modo protegido usam dinƒmico" },
			{ "dynamic", "Dinƒmico - Recompila‡„o dinƒmica (r pida, usando a implementa‡„o dynamic_x86)" },
			#elif defined(C_DYNREC)
			{ "auto", "Autom tico - Jogos em modo real usam normal, jogos em modo protegido usam dinƒmico" },
			{ "dynamic", "Dinƒmico - Recompila‡„o dinƒmica (r pida, usando a implementa‡„o dynrec)" },
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
		"Avan‡ado > Modifica‡”es no Disco do SO (necess rio reiniciar)", NULL,
		"Ao executar um sistema operacional instalado, as modifica‡”es na unidade C: ser„o feitas na imagem de disco por padr„o." "\n"
		"Definir para 'Descartar' permite que o conte£do seja fechado a qualquer momento sem preocupa‡”es com corrup‡„o do sistema de arquivos ou do registro." "\n"
		"Ao usar 'Salvar Diferen‡a por Conte£do', a imagem do disco nunca deve ser modificada novamente, caso contr rio, as diferen‡as existentes se tornar„o inutiliz veis.", NULL,
		DBP_OptionCat::System,
		{
			{ "false", "Manter (padr„o)" },
			{ "true", "Descartar" },
			{ "diff", "Salvar Diferen‡a por Conte£do" },
		},
		"false"
	},
	{
		"dosbox_pure_bootos_dfreespace",
		"Avan‡ado > Espa‡o Livre em D: no SO (necess rio reiniciar)", NULL,
		"Controla a quantidade de espa‡o livre dispon¡vel na unidade D: ao executar um sistema operacional instalado." "\n"
		"Se o tamanho total da unidade D: (dados + espa‡o livre) exceder 2 GB, n„o poder  ser usado nas vers”es anteriores do Windows 95." "\n"
		"ATEN€ŽO: Os arquivos de salvamento criados est„o vinculados a essa configura‡„o, portanto, alter -la ocultar  todas as altera‡”es existentes na unidade D:.", NULL,
		DBP_OptionCat::System,
		{ { "1024", "1GB (padr„o)" }, { "2048", "2GB" }, { "4096", "4GB" }, { "8192", "8GB" }, { "discard", "Descartar Altera‡”es em D:" }, { "hide", "Desativar Disco R¡gido D: (usar apenas CD-ROM)" } }, // <<< NOVA TRADU€ŽO
		"1024"
	},
	{
		"dosbox_pure_bootos_forcenormal",
		"Avan‡ado > For‡ar N£cleo Normal no SO", NULL,
		"O n£cleo normal pode ser mais est vel ao executar um sistema operacional instalado." "\n"
		"Isso pode ser ligado e desligado para contornar travamentos." "\n\n", NULL, //end of System > Advanced section
		DBP_OptionCat::System,
		{ { "false", "Desativado (padr„o)" }, { "true", "Ativado" } },
		"false"
	},

	// Audio
	#ifndef DBP_STANDALONE
	{
		"dosbox_pure_audiorate",
		"Taxa de Amostragem de †udio (necess rio reiniciar)", NULL,
		"Isso deve corresponder … configura‡„o de taxa de sa¡da de  udio do frontend (Hz).", NULL,
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
		"Latˆncia de †udio", NULL,
		"Se definido muito baixo, podem ocorrer falhas no  udio. O valor ‚ para processamento interno e a latˆncia percebida ser  maior.", NULL,
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
		"Configura‡”es do SoundBlaster", NULL,
		"Defina o endere‡o, interrup‡„o, DMA de 8 bits baixos e DMA de 16 bits altos.", NULL,
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
		"Sa¡da MIDI", NULL,
		"Selecione o arquivo SoundFont .SF2, arquivo .ROM ou interface usada para sa¡da MIDI." "\n"
		#ifndef DBP_STANDALONE
		"Para adicionar SoundFonts ou arquivos ROM, copie-os para o diret¢rio 'system' do frontend." "\n"
		"Para usar o driver MIDI do frontend, certifique-se de que ele esteja configurado corretamente."
		#else
		"Para adicionar SoundFonts ou arquivos ROM, copie-os para o diret¢rio 'system' do DOSBox Pure." "\n" // <<< NOVA TRADU€ŽO
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
		"Avan‡ado > Tipo de SoundBlaster", NULL,
		"Tipo de placa SoundBlaster emulada.", NULL,
		DBP_OptionCat::Audio,
		{
			{ "sb16", "SoundBlaster 16 (padr„o)" },
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
		"Avan‡ado > Modo Adlib/FM do SoundBlaster", NULL,
		"O modo de s¡ntese FM emulado pelo SoundBlaster. Todos os modos s„o compat¡veis com o Adlib, exceto o CMS.", NULL,
		DBP_OptionCat::Audio,
		{
			{ "auto",     "Autom tico (selecionar com base no tipo de SoundBlaster) (padr„o)" },
			{ "cms",      "CMS (Creative Music System / GameBlaster)" },
			{ "opl2",     "OPL-2 (AdLib / OPL-2 / Yamaha 3812)" },
			{ "dualopl2", "Dual OPL-2 (Dual OPL-2 usado pelo SoundBlaster Pro 1.0 para som est‚reo)" },
			{ "opl3",     "OPL-3 (AdLib / OPL-3 / Yamaha YMF262)" },
			{ "opl3gold", "OPL-3 Gold (AdLib Gold / OPL-3 / Yamaha YMF262)" },
			{ "none",     "Desativado" },
		},
		"auto"
	},
	{
		"dosbox_pure_sblaster_adlib_emu",
		"Avan‡ado > Provedor de Adlib SoundBlaster", NULL,
		"Provedor para a emula‡„o do Adlib. O padr„o possui boa qualidade e baixos requisitos de desempenho.", NULL,
		DBP_OptionCat::Audio,
		{
			{ "default", "Padr„o" },
			{ "nuked", "Alta qualidade Nuked OPL3" },
		},
		"default"
	},
	{
		"dosbox_pure_gus",
		"Avan‡ado > Habilitar Emula‡„o do Gravis Ultrasound (necess rio reiniciar)", NULL,
		"Habilitar emula‡„o do Gravis Ultrasound. As configura‡”es est„o fixadas em porta 0x240, IRQ 5, DMA 3." "\n"
		"Se a vari vel ULTRADIR precisar ser diferente do padr„o 'C:\\ULTRASND' vocˆ precisar  inserir 'SET ULTRADIR=...' na linha de comando ou em um arquivo em lote.", NULL,
		DBP_OptionCat::Audio,
		{ { "false", "Desativado (padr„o)" }, { "true", "Ativado" } },
		"false"
	},
	{
		"dosbox_pure_tandysound",
		"Avan‡ado > Habilitar Dispositivo de Som Tandy (rein¡cio necess rio)", NULL,
		"Habilita a emula‡„o do Dispositivo de Som Tandy mesmo quando executando sem a emula‡„o do Adaptador Gr fico Tandy.", NULL,
		DBP_OptionCat::Audio,
		{ { "auto", "Desativado (padr„o)" }, { "on", "Ativado" } },
		"auto"
	},
	{
		"dosbox_pure_swapstereo",
		"Avan‡ado > Trocar Canais Est‚reo", NULL,
		"Trocar o canal de  udio esquerdo e direito." "\n\n", NULL, //end of Audio > Advanced section
		DBP_OptionCat::Audio,
		{ { "false", "Desativado (padr„o)" }, { "true", "Ativado" } },
		"false"
	},

	{ NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL }
};
