// -- ライブラリ --
import ddf.minim.*;
import processing.serial.*;

// -- 変数宣言 --
Minim minim;
AudioPlayer horn;                     // 警笛
AudioPlayer hornRelease;              // 警笛終了
AudioPlayer buzzer;                   // 連絡ブザー
AudioPlayer doorOpen;                 // ドア開
AudioPlayer doorClose;                // ドア閉
AudioPlayer departureMelody;          // 発車メロディ
AudioPlayer mc;                       // マスコン操作音
AudioPlayer airZero;                  // ブレーキ緩解音

final int PLAY = 1;
final int STOP = 0;

/*
final int hornNum = 30;
final int hornReleaseNum = 31;
final int buzzerNum = 32;
final int doorOpenNum = 33;
final int doorCLseNum = 34;
*/

boolean hornFlag = false;

Serial myPort;                        // シリアル通信の確立

void setup() {
    size(600, 600);                                     // ウインドウサイズを設定
    minim = new Minim(this);                            // Minimオブジェクトを生成
    horn = minim.loadFile("horn2.wav");                 // 警笛読み込み
    hornRelease = minim.loadFile("horn2Release.wav");   // 警笛終了読み込み
    buzzer = minim.loadFile("buzzer1.wav");             // 連絡ブザー音読み込み
    doorOpen = minim.loadFile("Dooropn.wav");           // ドア開音読み込み
    doorClose = minim.loadFile("Doorcls.wav");          // ドア閉音読み込み
    departureMelody = minim.loadFile("nagoya.wav");       // 発車メロディ読み込み
    mc = minim.loadFile("notch.wav");                   // ノッチ操作音
    airZero = minim.loadFile("AirZero.wav");            // ブレーキ緩解音
    
    myPort = new Serial(this, "/dev/cu.usbmodem1421", 9600);
    myPort.clear();
}

void draw() {
    background(255);    
}

void serialEvent(Serial p) {
    String inString = myPort.readStringUntil('\n');     // Arduinoから文字列を受信
    
    if(inString != null) {
        inString = trim(inString);                      // 前後の空白の除去
        int data[] = int(split(inString, ','));             // コンマ区切りで読み込む
        
        if(data.length > 0) {
            // -- サウンドの再生 --
            // 警笛
            if (data[0] == PLAY) {
                if (hornFlag == false) {
                    hornFlag = true;
                    horn.play();                        // 警笛の再生 
                    hornRelease.rewind();
                }
            } else if (data[0] == STOP) {
                hornFlag = false;
                horn.pause();                           // 警笛の停止
                horn.rewind();
                hornRelease.play();                     // 警笛終了音の再生
            }
            
            // ブザー音
            if (data[1] == PLAY) {
                buzzer.play();                          // ブザー音の再生
            } else {
                buzzer.pause();                          // 一時停止
                buzzer.rewind();                         // 曲の頭に戻る
            }
            
            // ドア開
            if (data[2] == PLAY) {
                doorOpen.play();                        // ドア開音再生
                doorOpen.rewind();
            } else if (data[2] == STOP) {
            }
            
            
            // ドア閉
            if (data[3] == PLAY) {
                doorClose.play();
                doorClose.rewind();
            } else if(data[3] == STOP) {
            }
            
            // 発車メロディ
            if (data[4] == PLAY) {
                departureMelody.play();
                departureMelody.rewind();
            } else {
            }
            
            // マスコン操作音
            if (data[5] == PLAY) {
                mc.play();                        // ドア開音再生
                mc.rewind();
            } else if (data[5] == STOP) {
            }
            
            // ブレーキ緩解音
            if (data[6] == PLAY) {
                airZero.play();
                airZero.rewind();
            } else if (data[6] == STOP) {
                
            }
        }
    }
}

void stop() {
  horn.close();
  buzzer.close();
  doorClose.close();
  doorOpen.close();
  departureMelody.close();
  mc.close();
  minim.stop();
  super.stop();
}