/********************************************************/
/* Windows MCI を使用したMIDIメッセージ実行プログラム   */
/********************************************************/

/* CH 1 Acoustic Grand Piano (#1) */
/* CH 2 Violin (#41) */
/* CH10 リズムパート */

/* 'a' キー: CH1 ノートオン  */
/* 'z' キー: CH1 ノートオフ  */
/* 's' キー: CH2 ノートオン  */
/* 'x' キー: CH2 ノートオフ  */
/* 'd' キー: CH10 Chinese Cymbal ノートオン  */
/* 'c' キー: CH10 Chinese Cymbal ノートオフ  */
/* 'Q','q' キー: プログラム終了 */

/* MSYS2でのコンパイル方法 */
/* % cc midimain.c -lwinmm */

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

HMIDIOUT g_hMidiout;

int mygetch(void) {
  struct termios oldt,
                 newt;
  int            ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;
}

int mykbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
  
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
  
  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
  
  if (ch != EOF) {
    ungetc(ch, stdin);
    return 1;
  }
  
  return 0;
}

void Open_MCImidi(void)
{
  if (midiOutOpen(&g_hMidiout, MIDIMAPPER, 0, 0, 0) != MMSYSERR_NOERROR) {
    fprintf(stderr,"MCI OPEN FAILED\n");
    return;
  }
}

void Write_MCImidi(int num, unsigned char *buf)
{

  if(num>4){                    /* ショートメッセージか否か */
    MIDIHDR mhMidi;
    ZeroMemory(&mhMidi, sizeof(mhMidi));
    mhMidi.lpData = (LPSTR)buf;
    mhMidi.dwBufferLength = num;
    mhMidi.dwBytesRecorded = num;
    midiOutPrepareHeader(g_hMidiout, &mhMidi, sizeof(mhMidi));

    /* バッファ送信 */
    if(midiOutLongMsg(g_hMidiout, &mhMidi, sizeof(mhMidi))!=MMSYSERR_NOERROR){
      midiOutUnprepareHeader(g_hMidiout, &mhMidi, sizeof(mhMidi));
      fprintf(stderr,"送信失敗\n");
      return;
    }
    /* 送信完了待機 */
    while((mhMidi.dwFlags & MHDR_DONE)==0);
    midiOutUnprepareHeader(g_hMidiout, &mhMidi, sizeof(mhMidi));
  }else{
    /* ショートメッセージなら */
    union { 
        DWORD dwData; 
        BYTE bData[4]; 
    }u;
    u.bData[0] = buf[0];
    u.bData[1] = buf[1];
    u.bData[2] = buf[2];
    u.bData[3] = buf[3];
    midiOutShortMsg(g_hMidiout, u.dwData); 
  }
}

void Close_MCImidi(void)
{
  midiOutReset(g_hMidiout);
  midiOutClose(g_hMidiout);
}

void gm_reset(void)
{
  unsigned char buf[6]={0xf0, 0x7e, 0x7f, 0x09, 0x01, 0xf7};
  Write_MCImidi(6, buf);
}


int main(void){
  unsigned char buf[10];

  /* MIDIオープン */
  Open_MCImidi();

  /* GMリセット送信 */
  gm_reset();

  /* プログラムチェンジ */
  buf[0]=0xc0 | 0x00;           /* ステータスメッセージ: ch1 */
  buf[1]=0x00;                  /* データメッセージ: 0(Acoustic Grand Piano) */
  Write_MCImidi(2, buf);

  /* プログラムチェンジ */
  buf[0]=0xc0 | 0x01;           /* ステータスメッセージ: ch2 */
  buf[1]=0x29;                  /* データメッセージ: 41(Violin) */
  Write_MCImidi(2, buf);


  while(!mykbhit()){            /* キーボードが押されたら次に進む */
    switch(mygetch()){          /* 押されたキーボードによって分岐する */
    
    case 'q':                   /* q または Q キーが押された場合 */
    case 'Q':
      /* 終了処理 */
      Sleep(1000);              /* 1000ミリ秒待つ */
      Close_MCImidi();
      exit(1);

    case 'a':                   /* a キーが押された場合 */
      /* ch1 ノートオン */
      buf[0]=0x90 | 0x00;       /* ステータスメッセージ: ch1*/
      buf[1]=0x3c;              /* 音の高さ=60 */
      buf[2]=0x7f;              /* 音の強さ=127 */
      Write_MCImidi(3, buf);
      break;
    case 's':                   /* s キーが押された場合 */
      /* ch2ノートオン */
      buf[0]=0x90 | 0x01;       /* ステータスメッセージ: ch2*/
      buf[1]=0x3c;              /* 音の高さ=60 */
      buf[2]=0x7f;              /* 音の強さ=127 */
      Write_MCImidi(3, buf);
      break;
    case 'd':                   /* d キーが押された場合 */
      /* ch10 ノートオン:リズム音色 */
      buf[0]=0x90 | 0x09;       /* ステータスメッセージ: ch10*/
      buf[1]=0x34;              /* Chinese Cymbal = 0x34 */
      buf[2]=0x7f;              /* 音の強さ=127 */
      Write_MCImidi(3, buf);
      break;

    case 'z':                   /* z キーが押された場合 */
      /* ch1 ノートオフ */
      buf[0]=0x80 | 0x00;       /* ステータスメッセージ: ch1*/
      buf[1]=0x3c;
      buf[2]=0x7f;
      Write_MCImidi(3, buf);
      break;
    case 'x':                   /* z キーが押された場合 */
      /* ch2 ノートオフ */
      buf[0]=0x80 | 0x01;       /* ステータスメッセージ: ch2*/
      buf[1]=0x3c;
      buf[2]=0x7f;
      Write_MCImidi(3, buf);
      break;
    case 'c':                   /* c キーが押された場合 */
      /* ch10 ノートオフ:リズム音色 */
      buf[0]=0x80 | 0x09;       /* ステータスメッセージ: ch10*/
      buf[1]=0x34;              /* Chinese Cymbal = 0x34 */
      buf[2]=0x7f;              /* 音の強さ=127 */
      Write_MCImidi(3, buf);
      break;
    }
  }
  return 0;
}
