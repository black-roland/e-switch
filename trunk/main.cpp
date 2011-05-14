#include "..\include\Lib_Clara.h"
#include "..\include\dir.h"
#include "..\include\cfg_items.h"
#include "conf_loader.h"
#include "config_data.h"
#include "languages.h"

#define ELF_BCFG_CONFIG_EVENT 994

#define ELF_VERSION "2.0.2"

/* ----------- Прототипы функций ----------- */
int KeySwitch (int, int, int, LPARAM, DISP_OBJ*);
void CloseDesktop (int);
int RepeatELF (int, int, int, LPARAM, DISP_OBJ*);
bool CheckDesktop (int);
void RunFirstDesktop ();
void InitVariables ();
/* ----------------------------------------- */

/* ------ Глобальные переменные и пр. ------ */
struct ELF
{
  int Showing;
  const wchar_t* Path;
  const char* Book;
};

struct DESKTOP
{
  int Showing;
  ELF ELFs[5];
  const wchar_t* Wallpaper;
};

typedef struct
{
    BOOK* Book;
    wchar_t* BcfgEditPath;
    wchar_t* BcfgEditName;
}MSG_BCFG;

DESKTOP Desktops[5];
int ActiveDesktop = -1;
u16 timer;
/* ----------------------------------------- */

int IsOnStandby () // Проверка StandBy
{
    if (Display_GetTopBook(0) == Find_StandbyBook())
      return 1;
    
    return 0;
}

int Reconfig (void* mess, BOOK* book) // При реконфиге
{
  RECONFIG_EVENT_DATA *reconf = (RECONFIG_EVENT_DATA *)mess;
  int result = 0;
  
  if (wstrcmpi (reconf->path, successed_config_path) == 0 &&
      wstrcmpi (reconf->name, successed_config_name) == 0)
  {
    int oldBCFG_Settings_RepeatActiveELF = BCFG_Settings_RepeatActiveELF;
    
    CloseDesktop (ActiveDesktop);
    InitVariables ();
    RunFirstDesktop ();
    if (BCFG_Settings_RepeatActiveELF != oldBCFG_Settings_RepeatActiveELF)
      if (BCFG_Settings_RepeatActiveELF)
        ModifyKeyHook(RepeatELF, KEY_HOOK_ADD, 0);
      else
        ModifyKeyHook(RepeatELF, KEY_HOOK_REMOVE, 0);
    result = 1;
  }
  
  return result;
}

int OnBcfgConfig (void* Data, BOOK* Book) // Поддержка мода BookManager
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

void LoadLastState () // Загрузка состояния
{
  int f;
  wchar_t path[201];
  wstrcpy(path, GetDir(DIR_INI|MEM_INTERNAL));
  wstrcat(path, L"/E-Switch.ini");
  
  W_FSTAT stat;
  if (w_fstat(path, &stat) == 0)
  {
    if ((f = w_fopen(path, WA_Read, 0x1FF, 0)) >= 0)
    {
      char* buf = new char[stat.st_size+1];
      w_fread(f, buf, stat.st_size);
      w_fclose(f);
      char* adstr = new char[0];
      adstr = manifest_GetParam(buf, "[ActiveDesktop]", 0);
      int adint = adstr[0] - 0x30;
      delete(buf);
      if ((adint >= 0) && (adint <= 4))
        ActiveDesktop = adint;
    }
  }
}

void SaveLastState () // Сохранение состояния
{
  int f;
  wchar_t path[201];
  wstrcpy(path, GetDir(DIR_INI|MEM_INTERNAL));
  wstrcat(path, L"/E-Switch.ini");
  
  if((f = w_fopen(path, WA_Read+WA_Write+WA_Create+WA_Truncate, 0x1FF, 0)) >= 0)
  {
    char buf[22];
    sprintf(buf, "[ActiveDesktop]: %d\n", ActiveDesktop);
    w_fwrite(f, buf, strlen(buf));
    w_fclose(f);
  }
}

int OnPlayer (void*, BOOK* book) // При запуске проигрывателя
{
  if (BCFG_Settings_OnPlayerStart != 0)
  {
     CheckDesktop (BCFG_Settings_OnPlayerStart - 1);
  }
  
  return 1;
}

int OnPhonePowerDown (void*, BOOK* book) // При выключении телефона
{
  SaveLastState ();
  int book_id = BookObj_GetBookID(book);
  UI_CONTROLLED_SHUTDOWN_RESPONSE(book_id);
  
  return 1;
}

bool FileExist (const wchar_t* fullpath) // Проверка существования указанного файла
{
  W_FSTAT stat;
  
  if (w_fstat(fullpath, &stat) == 0)
    return true;
  
  return false;
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
  MessageBox(EMPTY_TEXTID, TextID_Create("E-switch\n" ELF_VERSION " " LNG_NAME "\n\n(c) Black_Roland\nE-mail:\nblack_roland@mail.ru", ENC_LAT1, TEXTID_ANY_LEN), NOIMAGE, 1, 5000, msg->book);
  
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
  ModifyKeyHook(KeySwitch, KEY_HOOK_REMOVE, 0);
  if (BCFG_Settings_RepeatActiveELF)
    ModifyKeyHook(RepeatELF, KEY_HOOK_REMOVE, 0);
  SaveLastState ();
  if (BCFG_Settings_ClsActELFWhnEswtchCls)
      CloseDesktop(ActiveDesktop);
  SUBPROC (elf_exit);
}

BOOK* CreateESBook ()
{
  ESBook = new BOOK;
  CreateBook(ESBook, onCloseESBook, &base_page, "E-switch", -1, 0);
  
  return (ESBook);
}
/* ----------------------------------------- */

int SetWallpaper (const wchar_t* fullpath) // Установка указанных обоев // Я бы сочинил про эту многострадальную функцию стихотворение, но не умею
{
  if (FileExist(fullpath))
  {  
    wchar_t* path = new wchar_t[201];
    wstrcpy (path, fullpath);
    wchar_t* fname = wstrrchr (path, '/') + 1;
    *wstrrchr (path, '/') = 0;
    
    if ((StandbyBackground_SetImage (3, 0, 0, path, fname, 0)) != 0)
      return 2;
  }
  else
    return 1;
  
  return 0;
}

int BookToFind (BOOK* book, int* param)
{
  if(!strcmp(book->xbook->name, (char*)param))
    return 1;
  
  return 0;
}

int CloseELF (ELF ELFObj) // Закрытие эльфа
{
  if (strcmp(ELFObj.Book, "E-switch")) // Защита от суицида
  {
    BOOK* bk = FindBookEx (BookToFind, (int*)ELFObj.Book);
    int bkid = BookObj_GetBookID (bk);
    
    if (bkid != -1) // Если книга найдена
      UI_Event_toBookID(ELF_TERMINATE_EVENT, bkid); // Закрыть эльф
    else
      return 2;
  }
  else
    return 1;
  
  return 0;
}

int RunELF (ELF ELFObj) // Запуск эльфа
{
  if (FileExist(ELFObj.Path)) // Если есть файл эльфа
  {
    if (elfload(ELFObj.Path, 0, 0, 0) != 0) // Запустить эльф
      return 2; // Если запуск неудачный
  }
  else
    return 1; // Если нет файла эльфа

  return 0;
}

void CloseDesktop (int n) // Закрытие рабочего стола
{
  int i;
  bool e = false;
  
  for (i = 0; i < 5; i++)
  {
    if (Desktops[n].ELFs[i].Showing == 1)
      if (CloseELF(Desktops[n].ELFs[i]) != 0)
        e = true;
  }
  if (e == true) // Если найдены ошибки
    MessageBox(EMPTY_TEXTID, STR(LNG_DESKTOP_CLOSE_ERROR), NOIMAGE, 1, 5000, 0);
}

void RunDesktop (int n) // Запуск рабочего стола
{
  int i;
  bool e = false;
  
  for (i = 0; i < 5; i++)
  {
    if (Desktops[n].ELFs[i].Showing == 1)
      if (RunELF(Desktops[n].ELFs[i]) != 0)
        e = true;
  }
  // Перерисовка StandBy
  DISP_OBJ* RedrawGUI = GUIObject_GetDispObject(SBY_GetStatusIndication(Find_StandbyBook()));
  DispObject_InvalidateRect(RedrawGUI, 0);
  if (e == true) // Если найдены ошибки
    MessageBox(EMPTY_TEXTID, STR(LNG_DESKTOP_RUN_ERROR), NOIMAGE, 1, 5000, 0);
}

void OnTimerRunDesktop (u16 timerID, LPARAM n) // Отложенный запуск рабочего стола
{
  RunDesktop(n);
  Timer_Kill(&timer);
}

bool CheckDesktop (int i) // Выполнение проверок и запуска/закрыия рабочих столов
{
  if (Desktops[i].Showing == 1)
  {
    CloseDesktop (ActiveDesktop); // Закрытие рабочего стола
    timer = Timer_Set(200, OnTimerRunDesktop, i); // Отложенный запуск рабочего стола
    ActiveDesktop = i;
    if (BCFG_22_WallWhenSwitch == 1) // Установка обоев
      if ((SetWallpaper (Desktops[i].Wallpaper)) != 0)
        MessageBox(EMPTY_TEXTID, STR(LNG_WALLPAPER_SET_ERROR), NOIMAGE, 1, 5000, 0);

    return true;
  }
  
  return false;
}

void VibrationNotify ()
{
  AudioControl_Vibrate(*GetAudioControlPtr(), BCFG_Settings_VibrationTime, 0, BCFG_Settings_VibrationTime);
}

int SoundNotify ()
{
  if (FileExist(BCFG_SoundPath))
  {
    wchar_t* path = new wchar_t[201];
    wstrcpy (path, BCFG_SoundPath);
    wchar_t* fname = wstrrchr (path, '/') + 1;
    *wstrrchr (path, '/') = 0;
    if (PlayFileV (path, fname, 100) == 0)
      return 2;
  }
  else
    return 1;
  
  return 0;
}

void AfterDesktopSwitch () // Функция выполняемая после переключения столов по клавише
{
  if (BCFG_Settings_VibrateWhenSwitch)
    VibrationNotify ();
  if (BCFG_SoundWhenSwitch)
    if (SoundNotify () != 0)
      MessageBox(EMPTY_TEXTID, STR(LNG_SOUND_PLAY_ERROR), NOIMAGE, 1, 5000, 0);
}

void NextDesktop ()// Переключить на следующий рабочий стол
{
  int i = ActiveDesktop;
  
  if (ActiveDesktop != -1)
  {
    do
    {
      if (i < 4)
        i++;
      else
        i = 0;
    }
    while (CheckDesktop (i) == false);
    AfterDesktopSwitch ();
  }
  else
    MessageBox(EMPTY_TEXTID, STR(LNG_ALL_ACTS_DISABLED), NOIMAGE, 1, 5000, 0);
}

void PreviousDesktop () // Переключить на предыдущий рабочий стол
{
  int i = ActiveDesktop;
  
  if (ActiveDesktop != -1)
  {
    do
    {
      if (i > 0)
        i--;
      else
        i = 4;
    }
    while (CheckDesktop (i) == false);
    AfterDesktopSwitch ();
  }
  else
    MessageBox(EMPTY_TEXTID, STR(LNG_ALL_ACTS_DISABLED), NOIMAGE, 1, 5000, 0);
}

int KeySwitch (int key, int r1 , int mode, LPARAM lparam, DISP_OBJ* dispobj) // Функиця переключающая столы по клавише
{
  if ((key == BCFG_Settings_Keys_ShiftForward_KeyCode) && (mode == BCFG_Settings_Keys_ShiftForward_KeyMode))
    if (!isKeylocked() && (BCFG_Settings_OnlyInStandBy ? IsOnStandby() : 1))
    {
      NextDesktop();
      
      return -1;
    }
  if ((key == BCFG_Settings_Keys_ShiftBackward_KeyCode) && (mode == BCFG_Settings_Keys_ShiftBackward_KeyMode))
    if (!isKeylocked() && (BCFG_Settings_OnlyInStandBy ? IsOnStandby() : 1))
    {
      PreviousDesktop();
      
      return -1;
    }
    
  return 0;
}

int RepeatELF (int key, int r1 , int mode, LPARAM lparam, DISP_OBJ* dispobj) // Повторный запуск эльфа по клавише // Иф иф иф иф иф...
{
  if ((key == BCFG_Settings_Keys_RepeatActiveELF_KeyCode) &&
      (mode == BCFG_Settings_Keys_RepeatActiveELF_KeyMode))
    if (!isKeylocked() && (BCFG_Settings_OnlyInStandBy ? IsOnStandby() : 1))
      if ((ActiveDesktop != -1) && (Desktops[ActiveDesktop].ELFs[4].Showing == 1))
        if (RunELF(Desktops[ActiveDesktop].ELFs[4]) != 0)
            MessageBox(EMPTY_TEXTID, STR(LNG_ERROR), NOIMAGE, 1, 5000, 0);
  
  return 0;
}

void RunFirstDesktop () // Запуск первого стола из списка
{
  if (ActiveDesktop == -1)
    ActiveDesktop = 0;
  int i = ActiveDesktop;
  
  while (Desktops[i].Showing == 0)
  {
    if (i < 4)
      i++;
    else
      i = 0;
    if (i == ActiveDesktop)
    {
      ActiveDesktop = -1;
      break;
    }
  }
  if (ActiveDesktop != -1)
    timer = Timer_Set(200, OnTimerRunDesktop, i);
}

void InitVariables () // Инициализация переменных
{
  InitConfig();
  const int BCFG_ShowingArray[5][5] = {
    {BCFG_11_ELF1_Showing, BCFG_11_ELF2_Showing, BCFG_11_ELF3_Showing, BCFG_11_ELF4_Showing, BCFG_11_ELF5_Showing},
    {BCFG_12_ELF1_Showing, BCFG_12_ELF2_Showing, BCFG_12_ELF3_Showing, BCFG_12_ELF4_Showing, BCFG_12_ELF5_Showing},
    {BCFG_13_ELF1_Showing, BCFG_13_ELF2_Showing, BCFG_13_ELF3_Showing, BCFG_13_ELF4_Showing, BCFG_13_ELF5_Showing},
    {BCFG_14_ELF1_Showing, BCFG_14_ELF2_Showing, BCFG_14_ELF3_Showing, BCFG_14_ELF4_Showing, BCFG_14_ELF5_Showing},
    {BCFG_15_ELF1_Showing, BCFG_15_ELF2_Showing, BCFG_15_ELF3_Showing, BCFG_15_ELF4_Showing, BCFG_15_ELF5_Showing}};
  const wchar_t* BCFG_PathArray[5][5] = {
    {BCFG_11_ELF1_ELFPath, BCFG_11_ELF2_ELFPath, BCFG_11_ELF3_ELFPath, BCFG_11_ELF4_ELFPath, BCFG_11_ELF5_ELFPath},
    {BCFG_12_ELF1_ELFPath, BCFG_12_ELF2_ELFPath, BCFG_12_ELF3_ELFPath, BCFG_12_ELF4_ELFPath, BCFG_12_ELF5_ELFPath},
    {BCFG_13_ELF1_ELFPath, BCFG_13_ELF2_ELFPath, BCFG_13_ELF3_ELFPath, BCFG_13_ELF4_ELFPath, BCFG_13_ELF5_ELFPath},
    {BCFG_14_ELF1_ELFPath, BCFG_14_ELF2_ELFPath, BCFG_14_ELF3_ELFPath, BCFG_14_ELF4_ELFPath, BCFG_14_ELF5_ELFPath},
    {BCFG_15_ELF1_ELFPath, BCFG_15_ELF2_ELFPath, BCFG_15_ELF3_ELFPath, BCFG_15_ELF4_ELFPath, BCFG_15_ELF5_ELFPath}};
  const char* BCFG_BookArray[5][5] = {
    {BCFG_11_ELF1_BookName, BCFG_11_ELF2_BookName, BCFG_11_ELF3_BookName, BCFG_11_ELF4_BookName, BCFG_11_ELF5_BookName},
    {BCFG_12_ELF1_BookName, BCFG_12_ELF2_BookName, BCFG_12_ELF3_BookName, BCFG_12_ELF4_BookName, BCFG_12_ELF5_BookName},
    {BCFG_13_ELF1_BookName, BCFG_13_ELF2_BookName, BCFG_13_ELF3_BookName, BCFG_13_ELF4_BookName, BCFG_13_ELF5_BookName},
    {BCFG_14_ELF1_BookName, BCFG_14_ELF2_BookName, BCFG_14_ELF3_BookName, BCFG_14_ELF4_BookName, BCFG_14_ELF5_BookName},
    {BCFG_15_ELF1_BookName, BCFG_15_ELF2_BookName, BCFG_15_ELF3_BookName, BCFG_15_ELF4_BookName, BCFG_15_ELF5_BookName}};
  
  int i, j;
  bool f;
  
  for (i = 0; i < 5; i++)
  {
    f = false;
    for (j = 0; j < 5; j++)
    {
      // Запись значений в массив
      Desktops[i].ELFs[j].Showing = (BCFG_ShowingArray[i][j] == 0 ? 1 : 0); // Чтобы не ломать мозг инверсией
      Desktops[i].ELFs[j].Path = BCFG_PathArray[i][j];
      Desktops[i].ELFs[j].Book = BCFG_BookArray[i][j];
      if (Desktops[i].ELFs[j].Showing == 1)
        f = true;
    }
    if (f == false) // Если все эльфы рабочего стола отключены, то отключаем раб. стол
      Desktops[i].Showing = 0;
    else
      Desktops[i].Showing = 1;
  }
  
  if (BCFG_22_WallWhenSwitch) // Обои водостойкие
  {
    const wchar_t* BCFG_WallArray[5] = {BCFG_221_Wall1, BCFG_221_Wall2, BCFG_221_Wall3, BCFG_221_Wall4, BCFG_221_Wall5};
    for (i = 0; i < 5; i++)
      Desktops[i].Wallpaper = BCFG_WallArray[i];
  }
}

void FirstRun () // Действия выполяемые при запуске
{
  InitVariables ();
  LoadLastState ();
  RunFirstDesktop ();
  AudioControl_Init();
  ModifyKeyHook(KeySwitch, KEY_HOOK_ADD, 0);
  if (BCFG_Settings_RepeatActiveELF)
    ModifyKeyHook(RepeatELF, KEY_HOOK_ADD, 0);
}

int main ()
{
  if (FindBookEx(BookToFind, (int*)"E-switch")) // Проверка на запущенность
  {
    MessageBox(EMPTY_TEXTID, STR(LNG_ALREADY_RUNNED), NOIMAGE, 1, 5000, 0);
    SUBPROC(elf_exit);
    
    return 0;
  }
  trace_init ();
  CreateESBook ();
  FirstRun ();
    
  return 0;
}
