#include "..\include\Lib_Clara.h"
#include "..\include\dir.h"
#include "conf_loader.h"
#include "config_data.h"
#include "languages.h"

#define ELF_BCFG_CONFIG_EVENT 994

#define ELF_VERSION L"1.3"

/* ----------- Прототипы функций ----------- */
int Switch (int, int, int, LPARAM, DISP_OBJ*);
void FirstRun ();
void CloseAndRun (int, int);
int RepeatELF (int, int, int, LPARAM, DISP_OBJ*);
int BookToFind (BOOK*, int*);
void ExecuteFirstELF ();
void InitVariables ();
/* ----------------------------------------- */

/* ------ Глобальные переменные и пр. ------ */
struct ELF
{
  int state;
  const wchar_t* path;
  const char* book;
};

typedef struct
{
    BOOK* Book;
    wchar_t* BcfgEditPath;
    wchar_t* BcfgEditName;
}MSG_BCFG;

ELF ELFs[5];
int ActiveELFNumber = -1;
/* ----------------------------------------- */

int IsOnStandby ()
{
    if (Display_GetTopBook(0) == Find_StandbyBook())
      return 1;
    
    return 0;
}

int Reconfig (void*, BOOK* book)
{
  ELF oldActiveELFNumber;
  if (ActiveELFNumber != -1)
    oldActiveELFNumber = ELFs[ActiveELFNumber];
  int oldBCFG_Settings_RepeatActiveELF = BCFG_Settings_RepeatActiveELF;
  InitVariables ();
  if (ActiveELFNumber != -1)
  {
    // Убийство старого эльфа если реконфиг и они отличаются
    if ((oldActiveELFNumber.state != ELFs[ActiveELFNumber].state) ||
        (oldActiveELFNumber.path != ELFs[ActiveELFNumber].path) ||
          (oldActiveELFNumber.book != ELFs[ActiveELFNumber].book))
    {
      if ((ActiveELFNumber != -1) && (oldActiveELFNumber.state != 2))
      {
        if (strcmp(oldActiveELFNumber.book, "E-switch") != 0) // Защита от суицида
        {
          BOOK* bk = FindBookEx (BookToFind, (int*)oldActiveELFNumber.book);
          int bkid = BookObj_GetBookID (bk);
          
          if (bkid != -1) // Если книга найдена
            UI_Event_toBookID(ELF_TERMINATE_EVENT, bkid); // Закрыть предыдущий эльф
        }
      }
      ExecuteFirstELF ();
    }
  }
  else
    ExecuteFirstELF ();
 
  if (BCFG_Settings_RepeatActiveELF != oldBCFG_Settings_RepeatActiveELF)
    if (BCFG_Settings_RepeatActiveELF)
      ModifyKeyHook(RepeatELF, KEY_HOOK_ADD, 0);
    else
      ModifyKeyHook(RepeatELF, KEY_HOOK_REMOVE, 0);
  
  return 1;
}

int OnBcfgConfig (void* Data, BOOK* Book)
{
    MSG_BCFG* Message = (MSG_BCFG*)Data;
    
    wchar_t* FullName = new wchar_t[256];
    wstrcpy(FullName, Message->BcfgEditPath);
    wstrcat(FullName, L"/");
    wstrcat(FullName, Message->BcfgEditName);
    elfload(FullName, (void*)successed_config_path, (void*)successed_config_name, 0);
    delete(FullName);
    
    return 1;
}

void LoadSaveLastState (int a)
{
  int f;
  wchar_t path[201];
  wstrcpy(path, GetDir(DIR_INI|MEM_INTERNAL));
  wstrcat(path, L"/E-Switch.ini");
  
  if (a == 0) // Чтение состояния
  {
    W_FSTAT stat;
    if(w_fstat(path, &stat) == 0)
    {
      if((f = w_fopen(path, WA_Read, 0x1FF, 0)) >= 0)
      {
        char* buf = new char[stat.st_size+1];
        w_fread(f, buf, stat.st_size);
        w_fclose(f);
        char* aen = new char[1];
        aen = manifest_GetParam(buf, "ActiveELFNumber", 0);
        delete(buf);
        int a = aen[0] - 0x30;
        delete(aen);
        if ((a >= 0) && (a <= 4))
          ActiveELFNumber = a;
      }
    }
  }
  else // Запись состояния
  {
    if((f = w_fopen(path, WA_Read+WA_Write+WA_Create+WA_Truncate, 0x1FF, 0)) >= 0)
    {
      char buf[22];
      sprintf(buf, "ActiveELFNumber: %d\n", ActiveELFNumber);
      w_fwrite(f, buf, strlen(buf));
      w_fclose(f);
    }
  }
}

int OnPlayer (void*, BOOK* book)
{
  if (BCFG_Settings_OnPlayerStart != 0)
  {
    switch (ELFs[(BCFG_Settings_OnPlayerStart - 1)].state)
    {
      case 1: // Эльф
      {
        CloseAndRun ((ELFs[ActiveELFNumber].state != 2 ? ActiveELFNumber : -1), (BCFG_Settings_OnPlayerStart - 1)); // Если не пустота, то закрыть
        ActiveELFNumber = (BCFG_Settings_OnPlayerStart - 1);
        
        break;
      }
      case 2: // Пустота
      {
        CloseAndRun ((ELFs[ActiveELFNumber].state != 2 ? ActiveELFNumber : -1), -1); // Если не пустота, то закрыть
        ActiveELFNumber = (BCFG_Settings_OnPlayerStart - 1);
        
        break;
      }
    }
  }
  
  return 1;
}

int OnPhonePowerDown (void*, BOOK* book)
{
  LoadSaveLastState (1);
  int book_id = BookObj_GetBookID(book);
  UI_CONTROLLED_SHUTDOWN_RESPONSE(book_id);
  
  return 1;
}

/* ------ Поддержка BookManager и пр. ------ */
BOOK * ESBook;

int TerminateElf (void * ,BOOK* book)
{
  FreeBook(book);
  
  return 1;
}

typedef struct
{
  BOOK * book;
}MSG;

int ShowAuthorInfo (void *mess ,BOOK* book)
{
  MSG * msg = (MSG*)mess;
  MessageBox(EMPTY_TEXTID, TextID_Create(_T("E-switch\n") ELF_VERSION _T(" ") LNG_NAME _T("\n\n(c) Black_Roland\nE-mail:\nblack_roland@mail.ru") LNG_ABOUT_TRANSLATOR, ENC_UCS2, TEXTID_ANY_LEN), 0, 1, 5000, msg->book);
  
  return 1;
}

const PAGE_MSG ES_PageEvents[]@ "DYN_PAGE" ={
  ELF_TERMINATE_EVENT,                        TerminateElf,
  ELF_SHOW_INFO_EVENT,                        ShowAuthorInfo,
  ELF_RECONFIG_EVENT,                         Reconfig,
  ELF_BCFG_CONFIG_EVENT,                      OnBcfgConfig,
  UI_CONTROLLED_SHUTDOWN_REQUESTED_EVENT_TAG, OnPhonePowerDown,
  UI_MEDIAPLAYER_CREATED_EVENT_TAG,           OnPlayer,
  NIL_EVENT_TAG,                              0
};

PAGE_DESC base_page ={"ES_BasePage", 0, ES_PageEvents};

void elf_exit(void)
{
  trace_done();
  kill_data(&ELF_BEGIN, (void(*)(void*))mfree_adr());
}

void onCloseESBook (BOOK* book)
{
  ModifyKeyHook(Switch, KEY_HOOK_REMOVE, 0);
  if (BCFG_Settings_RepeatActiveELF)
    ModifyKeyHook(RepeatELF, KEY_HOOK_REMOVE, 0);
  LoadSaveLastState (1);
  if ((BCFG_Settings_ClsActELFWhnEswtchCls) && (ELFs[ActiveELFNumber].state != 2))
      CloseAndRun(ActiveELFNumber, -1);
  SUBPROC (elf_exit);
}

BOOK* CreateESBook ()
{
  ESBook = new BOOK;
  CreateBook(ESBook, onCloseESBook, &base_page, "E-switch", -1, 0);
  
  return(ESBook);
}
/* ----------------------------------------- */

int BookToFind (BOOK* book, int* param)
{
  if(!strcmp(book->xbook->name, (char*)param))
    return 1;
  
  return 0;
}

void DelayedRun (u16 timerID , LPARAM n)
{
  if (n !=-1) // Запуск следующего эльфа
  {
    W_FSTAT stat;
    
    if (w_fstat(ELFs[n].path, &stat) == 0) // Если есть файл эльфа
    {
      if (elfload(ELFs[n].path, 0, 0, 0) != 0) // Запустить следующий эльф
        MessageBox(EMPTY_TEXTID, STR(LNG_ELF_START_ERROR), 0, 1, 5000, 0); // Если запуск неудачный
    }
    else
      MessageBox(EMPTY_TEXTID, STR(LNG_ELF_NOT_FOUND), 0, 1, 5000, 0); // Если нет файла эльфа
  }
  if (BCFG_Settings_RedrawStandByWhenSwitch) // Перерисовка StandBy
  {
    DISP_OBJ* RedrawGUI = GUIObject_GetDispObject(SBY_GetStatusIndication(Find_StandbyBook()));
    DispObject_InvalidateRect(RedrawGUI, 0);
  }
}

void CloseAndRun (int p, int n) // Функция переключения эльфов
{
  if (p !=-1) // Закрытие предыдущего эльфа
    if (strcmp(ELFs[p].book, "E-switch")) // Защита от суицида
    {
      BOOK* bk = FindBookEx (BookToFind, (int*)ELFs[p].book);
      int bkid = BookObj_GetBookID (bk);
      
      if (bkid != -1) // Если книга не найдена
        UI_Event_toBookID(ELF_TERMINATE_EVENT, bkid); // Закрыть предыдущий эльф
      else
        MessageBox(EMPTY_TEXTID, STR(LNG_BOOK_NOT_FOUND), 0, 1, 5000, 0);
    }
    else
      MessageBox(EMPTY_TEXTID, STR(LNG_INVALID_BOOK_NAME), 0, 1, 5000, 0);
    Timer_Set(100, DelayedRun, n);
}

int Switch (int key, int r1 , int mode, LPARAM lparam, DISP_OBJ* dispobj) // Обработчик клавиши, он же калькулятор эльфов
{
  if (((key == BCFG_Settings_Keys_ShiftForward_KeyCode) && (mode == BCFG_Settings_Keys_ShiftForward_KeyMode)) ||
      ((key == BCFG_Settings_Keys_ShiftBackward_KeyCode) && (mode == BCFG_Settings_Keys_ShiftBackward_KeyMode)))
    if (!isKeylocked() && (BCFG_Settings_OnlyInStandBy ? IsOnStandby() : 1))
    {
      if (ActiveELFNumber != -1) // Если все эльфы отключены
      {
        int i = ActiveELFNumber;
        bool f = false;
        
        do
        {
          if ((key == BCFG_Settings_Keys_ShiftForward_KeyCode) && (mode == BCFG_Settings_Keys_ShiftForward_KeyMode)) // Если переключение вперед 
            if (i < 4)
              i++;
            else
              i = 0;
          else // Иначе назад
            if (i > 0)
              i--;
            else
              i = 4;
          switch (ELFs[i].state)
          {
            case 1: // Эльф
            {
              CloseAndRun ((ELFs[ActiveELFNumber].state != 2 ? ActiveELFNumber : -1), i); // Если не пустота, то закрыть
              ActiveELFNumber = i;
              if (BCFG_Settings_VibrateWhenSwitch)
                AudioControl_Vibrate(*GetAudioControlPtr(), BCFG_Settings_VibrationTime, 0, BCFG_Settings_VibrationTime);
              f = true;
              
              break;
            }
            case 2: // Пустота
            {
              CloseAndRun ((ELFs[ActiveELFNumber].state != 2 ? ActiveELFNumber : -1), -1); // Если не пустота, то закрыть
              ActiveELFNumber = i;
              if (BCFG_Settings_VibrateWhenSwitch)
                AudioControl_Vibrate(*GetAudioControlPtr(), BCFG_Settings_VibrationTime, 0, BCFG_Settings_VibrationTime);
              f = true;
              
              break;
            }
          }
        }
        while (!f);
      }
      else
        MessageBox(EMPTY_TEXTID, STR(LNG_ALL_ACTS_DISABLED), 0, 1, 5000, 0);
      
      return -1;
    }
  
  return 0;
}

int RepeatELF (int key, int r1 , int mode, LPARAM lparam, DISP_OBJ* dispobj)
{
  if ((key == BCFG_Settings_Keys_RepeatActiveELF_KeyCode) &&
      (mode == BCFG_Settings_Keys_RepeatActiveELF_KeyMode))
  {
    if (!isKeylocked() && (BCFG_Settings_OnlyInStandBy ? IsOnStandby() : 1))
    {
      if ((ActiveELFNumber != -1) && (ELFs[ActiveELFNumber].state != 2)) // Если эльф(ы) отключен(ы)
      {
        CloseAndRun (-1, ActiveELFNumber);
        
        return -1;
      }
    }
  }
  
  return 0;
}

void ExecuteFirstELF () // Запуск первого подходящего эльфа
{
    if (ActiveELFNumber == -1)
      ActiveELFNumber = 0;
    int i = ActiveELFNumber;
    bool f = false;
    
    do
    {
      switch (ELFs[i].state)
      {
        case 1: // Эльф
        {
          CloseAndRun (-1, i);
          ActiveELFNumber = i;
          f = true;
          
          break;
        }
        case 2: // Пустота
        {
          ActiveELFNumber = i;
          f = true;
          
          break;
        }
      }
      if (!f) // Если действие выполнено, то не производим проверок
      {
        if (i < 4)
          i++;
        else
          i = 0;
        if (i == ActiveELFNumber) // Если прошли весь список
        {
          ActiveELFNumber = -1;
          f = true;
          #ifndef NDEBUG
            MessageBox(EMPTY_TEXTID, STR(LNG_ALL_ACTS_DISABLED), 0, 1, 5000, 0);
          #endif
        }
      }
    }
    while (!f);
}

void InitVariables ()
{
  InitConfig();
  // Чтение конфига и запись его в массив
  ELFs[0].state = BCFG_ELFs_ELF1_Action;
  ELFs[0].path  = BCFG_ELFs_ELF1_ELFPath;
  ELFs[0].book  = BCFG_ELFs_ELF1_BookName;
  ELFs[1].state = BCFG_ELFs_ELF2_Action;
  ELFs[1].path  = BCFG_ELFs_ELF2_ELFPath;
  ELFs[1].book  = BCFG_ELFs_ELF2_BookName;
  ELFs[2].state = BCFG_ELFs_ELF3_Action;
  ELFs[2].path  = BCFG_ELFs_ELF3_ELFPath;
  ELFs[2].book  = BCFG_ELFs_ELF3_BookName;
  ELFs[3].state = BCFG_ELFs_ELF4_Action;
  ELFs[3].path  = BCFG_ELFs_ELF4_ELFPath;
  ELFs[3].book  = BCFG_ELFs_ELF4_BookName;
  ELFs[4].state = BCFG_ELFs_ELF5_Action;
  ELFs[4].path  = BCFG_ELFs_ELF5_ELFPath;
  ELFs[4].book  = BCFG_ELFs_ELF5_BookName;
}

void FirstRun ()
{
  InitVariables ();
  LoadSaveLastState(0);
  ExecuteFirstELF ();
  AudioControl_Init();
  ModifyKeyHook(Switch, KEY_HOOK_ADD, 0);
  if (BCFG_Settings_RepeatActiveELF)
    ModifyKeyHook(RepeatELF, KEY_HOOK_ADD, 0);
}

int main ()
{
  if (FindBookEx(BookToFind, (int*)"E-switch")) // Проверка на запущенность
  {
    MessageBox(EMPTY_TEXTID, STR(LNG_ALREADY_RUNNED), 0, 1, 5000, 0);
    SUBPROC(elf_exit);
    
    return 0;
  }
  trace_init();
  CreateESBook();
  FirstRun ();
  
  return 0;
}
