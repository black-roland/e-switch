#include "..\include\Lib_Clara.h"
#include "..\include\dir.h"
#include "conf_loader.h"
#include "config_data.h"
#include "languages.h"

#define ELF_BCFG_CONFIG_EVENT 994

#define ELF_VERSION L"1.1"

/* ----------- ��������� ������� ----------- */
int Switch (int, int, int);
void FirstRun ();
void CloseAndRun (int, int);
int RepeatELF (int, int, int);
/* ----------------------------------------- */

/* ------ ���������� ���������� � ��. ------ */
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

void DelayedFirstRun (u16 timerID, void*)
{
  FirstRun();
}

int Reconfig (void*, BOOK* book)
{
  if (ELFs[ActiveELFNumber].state != 2)
      CloseAndRun(ActiveELFNumber, -1);
  ModifyKeyHook(RepeatELF, 0);
  Timer_Set(500, DelayedFirstRun, 0);
  
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
    
    return 1;
}

void LoadSaveLastState (int a)
{
  int f;
  wchar_t path[201];
  wstrcpy(path, GetDir(DIR_INI|MEM_INTERNAL));
  wstrcat(path, L"/E-Switch.ini");
  
  if (a == 0) // ������ ���������
  {
    W_FSTAT stat;
    if(w_fstat(path, &stat) == 0)
    {
      if((f = w_fopen(path, WA_Read, 0x1FF, 0)) >= 0)
      {
        char* buf = new char[stat.st_size+1];
        w_fread(f, buf, stat.st_size);
        w_fclose(f);
        char* v = new char;
        v = manifest_GetParam(buf, "[ActiveELFNumber]", 0);
        switch (v[0]) // C ���� ���� ���-�� �������, ���� �������� �������� ������
        {
          case '0':
          {
            ActiveELFNumber = 0;
            
            break;
          }
          case '1':
          {
            ActiveELFNumber = 1;
            
            break;
          }
          case '2':
          {
            ActiveELFNumber = 2;
            
            break;
          }
          case '3':
          {
            ActiveELFNumber = 3;
            
            break;
          }
          case '4':
          {
            ActiveELFNumber = 4;
            
            break;
          }
          default: ActiveELFNumber = -1;
        }
      }
    }
  }
  else // ������ ���������
  {
    if((f = w_fopen(path, WA_Read+WA_Write+WA_Create+WA_Truncate, 0x1FF, 0)) >= 0)
    {
      char buf[22];
      sprintf(buf, "[ActiveELFNumber]: %d\n", ActiveELFNumber);
      w_fwrite(f, buf, strlen(buf));
      w_fclose(f);
    }
  }
}

int OnPhonePowerDown (void*, BOOK* book)
{
  LoadSaveLastState (1);
  int book_id = BookObj_GetBookID(book);
  UI_CONTROLLED_SHUTDOWN_RESPONSE(book_id);
  
  return 1;
}

/* ------ ��������� BookManager � ��. ------ */
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
  MessageBox(EMPTY_SID, Str2ID(_T("E-switch\n") ELF_VERSION _T(" ") LNG_NAME _T("\n\n(c) Black_Roland\nE-mail:\nblack_roland@mail.ru") LNG_ABOUT_TRANSLATOR, 0, SID_ANY_LEN), 0, 1, 5000, msg->book);
  
  return 1;
}

const PAGE_MSG ES_PageEvents[]@ "DYN_PAGE" ={
  ELF_TERMINATE_EVENT,                        TerminateElf,
  ELF_SHOW_INFO_EVENT,                        ShowAuthorInfo,
  ELF_RECONFIG_EVENT,                         Reconfig,
  ELF_BCFG_CONFIG_EVENT,                      OnBcfgConfig,
  UI_CONTROLLED_SHUTDOWN_REQUESTED_EVENT_TAG, OnPhonePowerDown,
  NIL_EVENT_TAG,                              0
};

PAGE_DESC base_page ={"ES_BasePage", 0, ES_PageEvents};

void elf_exit(void)
{
  kill_data(&ELF_BEGIN, (void(*)(void*))mfree_adr());
}

void onCloseESBook (BOOK* book)
{
  ModifyKeyHook(Switch, 0);
  ModifyKeyHook(RepeatELF, 0);
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

void CloseAndRun (int p, int n) // ������� ������������ ������
{
  if (p !=-1) // �������� ����������� �����
    if (strcmp(ELFs[p].book, "E-switch")) // ������ �� �������
    {
      BOOK* bk = FindBookEx (BookToFind, (int*)ELFs[p].book);
      int bkid = BookObj_GetBookID (bk);
      
      if (bkid != -1) // ���� ����� �� �������
        UI_Event_toBookID(ELF_TERMINATE_EVENT, bkid); // ������� ���������� ����
      else
        MessageBox(EMPTY_SID, STR(LNG_BOOK_NOT_FOUND), 0, 1, 5000, 0);
    }
    else
      MessageBox(EMPTY_SID, STR(LNG_INVALID_BOOK_NAME), 0, 1, 5000, 0);
  if (n !=-1) // ������ ���������� �����
  {
    W_FSTAT stat;
    
    if (w_fstat(ELFs[n].path, &stat) == 0) // ���� ���� ���� �����
    {
      if (elfload(ELFs[n].path, 0, 0, 0) != 0) // ��������� ��������� ����
        MessageBox(EMPTY_SID, STR(LNG_ELF_START_ERROR), 0, 1, 5000, 0); // ���� ������ ���������
    }
    else
      MessageBox(EMPTY_SID, STR(LNG_ELF_NOT_FOUND), 0, 1, 5000, 0); // ���� ��� ����� �����
  }
  if (BCFG_Settings_RedrawStandByWhenSwitch) // ����������� StandBy
  {
    DISP_OBJ* RedrawGUI = GUIObject_GetDispObject(SBY_GetStatusIndication(Find_StandbyBook())); // �� ����������, �� � ���������� ���������� �� ���������
    DispObject_InvalidateRect(RedrawGUI, 0);
  }
}

int Switch (int key, int r1, int mode) // ���������� �������, �� �� ����������� ������
{
  if ((key == BCFG_Settings_SwitchingKey_KeyCode) &&
      (mode == BCFG_Settings_SwitchingKey_KeyMode))
    if (!isKeylocked() && (BCFG_Settings_OnlyInStandBy ? IsOnStandby() : 1))
    {
      if (ActiveELFNumber != -1) // ���� ��� ����� ���������
      {
        int i = ActiveELFNumber;
        bool f = false;
        
        do
        {
          if (i < 4)
            i++;
          else
            i = 0;
          switch (ELFs[i].state)
          {
            case 1: // ����
            {
              CloseAndRun ((ELFs[ActiveELFNumber].state != 2 ? ActiveELFNumber : -1), i); // ���� �� �������, �� �������
              ActiveELFNumber = i;
              if (BCFG_Settings_VibrateWhenSwitch)
                AudioControl_Vibrate(*GetAudioControlPtr(), 500, 0, 500);
              f = true;
              
              break;
            }
            case 2: // �������
            {
              CloseAndRun ((ELFs[ActiveELFNumber].state != 2 ? ActiveELFNumber : -1), -1); // ���� �� �������, �� �������
              ActiveELFNumber = i;
              if (BCFG_Settings_VibrateWhenSwitch)
                AudioControl_Vibrate(*GetAudioControlPtr(), 500, 0, 500);
              f = true;
              
              break;
            }
          }
        }
        while (!f);
      }
      else
        MessageBox(EMPTY_SID, STR(LNG_ALL_ACTS_DISABLED), 0, 1, 5000, 0);
      
      return -1;
    }
  
  return 0;
}

int RepeatELF (int key, int r1, int mode)
{
  if ((key == BCFG_Settings_RepeatActiveELF_KeyCode) &&
      (mode == BCFG_Settings_RepeatActiveELF_KeyMode))
  {
    if (!isKeylocked() && (BCFG_Settings_OnlyInStandBy ? IsOnStandby() : 1))
    {
      if (ActiveELFNumber != -1) // ���� ��� ����� ���������
      {
        if (ELFs[ActiveELFNumber].state != 2)
          CloseAndRun (-1, ActiveELFNumber);
        
        return -1;
      }
    }
  }
  
  return 0;
}

void FirstRun () // ������ �������� � ������ ������� ��������� �������� �� ������
{
  InitConfig();
  // ������ ������� � ������ ��� � ������
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
  // ������ ������� ����������� �����
  if (ActiveELFNumber == -1)
    ActiveELFNumber = 0;
  int i = ActiveELFNumber;
  bool f = false;
  
  do
  {
    switch (ELFs[i].state)
    {
      case 1: // ����
      {
        CloseAndRun (-1, i);
        ActiveELFNumber = i;
        f = true;
        
        break;
      }
      case 2: // �������
      {
        ActiveELFNumber = i;
        f = true;
        
        break;
      }
    }
    if (!f) // ���� �������� ���������, �� �� ���������� ��������
    {
      if (i < 4)
        i++;
      else
        i = 0;
      if (i == ActiveELFNumber)
      {
        ActiveELFNumber = -1;
        f = true;
        #ifndef NDEBUG
          MessageBox(EMPTY_SID, STR(LNG_ALL_ACTS_DISABLED), 0, 1, 5000, 0);
        #endif
      }
    }
  }
  while (!f);
  // ��������� ������
  if (BCFG_Settings_RepeatActiveELF)
    ModifyKeyHook(RepeatELF, 1);
}

int main ()
{
  if (FindBookEx(BookToFind, (int*)"E-switch")) // �������� �� ������������
  {
    MessageBox(EMPTY_SID, STR(LNG_ALREADY_RUNNED), 0, 1, 5000, 0);
    SUBPROC(elf_exit);
    
    return 0;
  }
  CreateESBook();
  LoadSaveLastState(0);
  /*#ifndef NDEBUG
    Timer_Set(500, DelayedFirstRun, 0);
  #endif
  #ifdef NDEBUG
    FirstRun ();
  #endif*/
  FirstRun ();
  AudioControl_Init();
  ModifyKeyHook(Switch, 1);
  
  return 0;
}
