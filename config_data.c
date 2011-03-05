#include "..\\include\cfg_items.h"
#include "config_data.h"
#include "languages.h"

#pragma diag_suppress = Pe177

__root const CFG_HDR cfghdr1 = { CFG_LEVEL ,LNG_ELFS , 1 , 0}; // arg3 = Begin Level >>>LevelName=ELFs;
  __root const CFG_HDR cfghdr2 = { CFG_LEVEL ,LNG_ELF1 , 1 , 0}; // arg3 = Begin Level >>>LevelName=ELF1;
    __root const CFG_HDR cfghdr3 = { CFG_CBOX , LNG_ACTION , 0 , 3 }; 
    __root const int BCFG_ELFs_ELF1_Action = 0; // Default item (Disabled).
    __root const CFG_CBOX_ITEM cfgRadioBtn3[3] = {LNG_DISABLED , LNG_SWITCH_TO_ELF , LNG_BLANK};
    __root const CFG_HDR cfghdr4 = { CFG_UTF16_STRING , LNG_ELF_PATH , 0 , 200};
    __root const wchar_t BCFG_ELFs_ELF1_ELFPath[201] = L""; // Default. 
    __root const CFG_HDR cfghdr5 = { CFG_STR_WIN1251 , LNG_BOOK_NAME , 0 , 200};
    __root const char BCFG_ELFs_ELF1_BookName[201] = ""; // Default. 
  __root const CFG_HDR cfghdr7 = {CFG_LEVEL , "" , 0 , 0 }; // arg3 = End Level
  
  __root const CFG_HDR cfghdr6 = { CFG_LEVEL ,LNG_ELF2 , 1 , 0}; // arg3 = Begin Level >>>LevelName=ELF2;
    __root const CFG_HDR cfghdr8 = { CFG_CBOX , LNG_ACTION , 0 , 3 }; 
    __root const int BCFG_ELFs_ELF2_Action = 0; // Default item (Disabled).
    __root const CFG_CBOX_ITEM cfgRadioBtn8[3] = {LNG_DISABLED , LNG_SWITCH_TO_ELF , LNG_BLANK};
    __root const CFG_HDR cfghdr9 = { CFG_UTF16_STRING , LNG_ELF_PATH , 0 , 200};
    __root const wchar_t BCFG_ELFs_ELF2_ELFPath[201] = L""; // Default. 
    __root const CFG_HDR cfghdr10 = { CFG_STR_WIN1251 , LNG_BOOK_NAME , 0 , 200};
    __root const char BCFG_ELFs_ELF2_BookName[201] = ""; // Default. 
  __root const CFG_HDR cfghdr12 = {CFG_LEVEL , "" , 0 , 0 }; // arg3 = End Level

  __root const CFG_HDR cfghdr11 = { CFG_LEVEL ,LNG_ELF3 , 1 , 0}; // arg3 = Begin Level >>>LevelName=ELF3;
    __root const CFG_HDR cfghdr13 = { CFG_CBOX , LNG_ACTION , 0 , 3 }; 
    __root const int BCFG_ELFs_ELF3_Action = 0; // Default item (Disabled).
    __root const CFG_CBOX_ITEM cfgRadioBtn13[3] = {LNG_DISABLED , LNG_SWITCH_TO_ELF , LNG_BLANK};
    __root const CFG_HDR cfghdr14 = { CFG_UTF16_STRING , LNG_ELF_PATH , 0 , 200};
    __root const wchar_t BCFG_ELFs_ELF3_ELFPath[201] = L""; // Default. 
    __root const CFG_HDR cfghdr15 = { CFG_STR_WIN1251 , LNG_BOOK_NAME , 0 , 200};
    __root const char BCFG_ELFs_ELF3_BookName[201] = ""; // Default. 
  __root const CFG_HDR cfghdr17 = {CFG_LEVEL , "" , 0 , 0 }; // arg3 = End Level

  __root const CFG_HDR cfghdr16 = { CFG_LEVEL ,LNG_ELF4 , 1 , 0}; // arg3 = Begin Level >>>LevelName=ELF4;
    __root const CFG_HDR cfghdr18 = { CFG_CBOX , LNG_ACTION , 0 , 3 }; 
    __root const int BCFG_ELFs_ELF4_Action = 0; // Default item (Disabled).
    __root const CFG_CBOX_ITEM cfgRadioBtn18[3] = {LNG_DISABLED , LNG_SWITCH_TO_ELF , LNG_BLANK};
    __root const CFG_HDR cfghdr19 = { CFG_UTF16_STRING , LNG_ELF_PATH , 0 , 200};
    __root const wchar_t BCFG_ELFs_ELF4_ELFPath[201] = L""; // Default. 
    __root const CFG_HDR cfghdr20 = { CFG_STR_WIN1251 , LNG_BOOK_NAME , 0 , 200};
    __root const char BCFG_ELFs_ELF4_BookName[201] = ""; // Default. 
  __root const CFG_HDR cfghdr22 = {CFG_LEVEL , "" , 0 , 0 }; // arg3 = End Level

  __root const CFG_HDR cfghdr21 = { CFG_LEVEL ,LNG_ELF5 , 1 , 0}; // arg3 = Begin Level >>>LevelName=ELF5;
    __root const CFG_HDR cfghdr23 = { CFG_CBOX , LNG_ACTION , 0 , 3 }; 
    __root const int BCFG_ELFs_ELF5_Action = 0; // Default item (Disabled).
    __root const CFG_CBOX_ITEM cfgRadioBtn23[3] = {LNG_DISABLED , LNG_SWITCH_TO_ELF , LNG_BLANK};
    __root const CFG_HDR cfghdr24 = { CFG_UTF16_STRING , LNG_ELF_PATH , 0 , 200};
    __root const wchar_t BCFG_ELFs_ELF5_ELFPath[201] = L""; // Default. 
    __root const CFG_HDR cfghdr25 = { CFG_STR_WIN1251 , LNG_BOOK_NAME , 0 , 200};
    __root const char BCFG_ELFs_ELF5_BookName[201] = ""; // Default. 
  __root const CFG_HDR cfghdr27 = {CFG_LEVEL , "" , 0 , 0 }; // arg3 = End Level
__root const CFG_HDR cfghdr28 = {CFG_LEVEL , "" , 0 , 0 }; // arg3 = End Level

__root const CFG_HDR cfghdr26 = { CFG_LEVEL ,LNG_SETTINGS , 1 , 0}; // arg3 = Begin Level >>>LevelName=Settings;
  __root const CFG_HDR cfghdr29 = {CFG_KEYCODE , LNG_SWITCH_KEY, 0 , 0};
  __root const int BCFG_Settings_SwitchingKey_KeyCode = 8; // Default -> Joy Center. 
  __root const int BCFG_Settings_SwitchingKey_KeyMode = 1; // Default -> @KeyMode@.
  __root const CFG_HDR cfghdr30 = {CFG_CHECKBOX , LNG_ONLY_IN_SB , 0 , 0 }; 
  __root const int BCFG_Settings_OnlyInStandBy = 1; // Default (True). 
  __root const CFG_HDR cfghdr31 = {CFG_CHECKBOX , LNG_VIBRATE , 0 , 0 }; 
  __root const int BCFG_Settings_VibrateWhenSwitch = 0; // Default (False). 
  __root const CFG_HDR cfghdr32 = {CFG_CHECKBOX , LNG_REDRAW_SB , 0 , 0 }; 
  __root const int BCFG_Settings_RedrawStandByWhenSwitch = 1; // Default (True). 
  __root const CFG_HDR cfghdr33 = {CFG_CHECKBOX , LNG_CLS_A_W_C , 0 , 0 };
  __root const int BCFG_Settings_ClsActELFWhnEswtchCls = 0; // Default (True). 
  __root const CFG_HDR cfghdr34 = {CFG_CHECKBOX , LNG_REPEAT , 0 , 0 };
  __root const int BCFG_Settings_RepeatActiveELF = 0; // Default (True). 
  __root const CFG_HDR cfghdr35 = {CFG_KEYCODE , LNG_REPEAT_KEY, 0 , 0};
  __root const int BCFG_Settings_RepeatActiveELF_KeyCode = 5;
  __root const int BCFG_Settings_RepeatActiveELF_KeyMode = 1;
__root const CFG_HDR cfghdr36 = {CFG_LEVEL , "" , 0 , 0 }; // arg3 = End Level
