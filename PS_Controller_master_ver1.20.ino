//====================================================================
// 仕様
//====================================================================
/*
配列se[]の割当
0 = 警笛
1 = ブザー
2 = ドア開
3 = ドア閉
4 = 発車メロディ
5 = ノッチ操作音
6 = ブレーキ緩解音

*/

//====================================================================
// ヘッダファイル
//====================================================================
#include <GPSXClass.h>

//====================================================================
//  定義マクロ
//====================================================================
#define PWMOUT 3                             // PWM出力ピン番号
#define PWMIN  4                             // PWM入力ピン番号
#define BUF 7                                // 音配列の大きさ
#define STOP 0                               // 音停止
#define PLAY 1                               // 音再生

//====================================================================
// 構造体宣言と列挙体
//====================================================================
//---- ハンドルポジション
// Br1はB1であるが、何故かコンパイルエラーになるのでB1のみBr1と記述する。
// 5ノッチ,4ノッチ ... ノッチオフ,1ブレーキ,2ブレーキ ... 8ブレーキ,非常ブレーキ
enum Position { N5, N4, N3, N2, N1, OFF, Br1, B2, B3, B4, B5, B6, B7, B8, EB };

//---- ノッチの構造体(ノッチとは車で言うアクセルのこと)
struct Notch
{
    int n1;
    int n2;
    int n3;
    int n4;
};

//---- ブレーキの構造体
struct Brake
{
    int b1;
    int b2;
    int b3;
    int b4;
};

//====================================================================
// 変数の大域宣言
//====================================================================
// ---- 多重再生防止用bool変数 ----
bool hornFlag = false;                // 警笛フラグ
bool buzzerFlag = false;              // ブザーフラグ
bool doorOpenFlag = false;            // ドア開フラグ
bool doorCloseFlag = false;           // ドア閉フラグ
bool departureMusicFlag = false;      // 発車メロディフラグ

int k;                                // 反復変数
int se[BUF];                          // 音再生要求配列
int cnt = 0;                          // 加減速度
double speed = 0.0;                   // スピード

Position tmpPos = OFF;                      // マスコン操作音を出すのに必要

//====================================================================
// 関数宣言
//====================================================================
Position button_down(void);                              // ハンドルポジションの格納
Position handle_pos( struct Notch n, struct Brake b );   // ハンドル位置の返却(ノッチの構造体, ブレーキの構造体)
double return_speedVal( Position pos, double speed );    // 速度の返却(ハンドルの位置, スピード)


//====================================================================
// setup()
//====================================================================
void setup() {
    Serial.begin(9600);                                         // シリアル通信 9600bps
    PSX.mode(PSX_PAD1, MODE_ANALOG, MODE_LOCK);                 // モード設定 PAD1を使用 アナログモード モードロック
    PSX.motorEnable(PSX_PAD1, MOTOR1_DISABLE, MOTOR2_DISABLE);  // モーターの無効化
    PSX.updateState(PSX_PAD1);
    // 3ピンからpwm出力
    pinMode( PWMOUT, OUTPUT );
    // 4ピンは入力
    pinMode( PWMIN, OUTPUT );
}

//====================================================================
// loop()
//====================================================================
void loop() {
    Position pos;                                              // ハンドルポジション
    pos = button_down();                                       // ハンドルポジションの取得

    if (cnt == 2) {
        speed = return_speedVal( pos, speed );                // ハンドル位置からスピードの増減を計算
        analogWrite( PWMOUT, speed );                         // PWM出力
        digitalWrite( PWMIN, LOW );                           // PWM入力
        cnt = 0;                                              // cntのリセット
    } else {
        cnt++;                                                // cntの加算
    }
    
    

    // ---- Processingに音再生の要求 ----
    for(k = 0; k < BUF; k++) {
        Serial.print(se[k]);                // Processingに音再生を要求
        if( BUF - k != 1 ) {                // 配列が残り1個のときはコンマを付けない
            Serial.print(",");              // split用コンマ
        }
    }
    Serial.print("\n");                     // 改行コード(Processing側でのSplitに必要)

    delay(5);                              // チャタリング防止ループ
}

//====================================================================
// ハンドルのビットの格納
//====================================================================
Position button_down()
{
    Notch notch;                                               // ノッチ型変数の宣言
    Brake bra;                                                 // ブレーキ型変数の宣言
    Position pos;                                              // ハンドルポジション

    PSX.updateState(PSX_PAD1);                                 // ハンドルポジションの取得の開始(これを書かないと取得不可)

    // -------- マスコン値の取得と格納 --------
    // ノッチ1
    if ( IS_DOWN_LEFT(PSX_PAD1) ) {
        notch.n1 = 1;
    } else {
        notch.n1 = 0;
    }

    // ノッチ2
    if ( IS_DOWN_DOWN(PSX_PAD1) ) {
        notch.n2 = 1;
    } else {
        notch.n2 = 0;
    }

    // ノッチ3
    if ( IS_DOWN_RIGHT(PSX_PAD1) ) {
        notch.n3 = 1;
    } else {
        notch.n3 = 0;
    }

    // ノッチ4
    if ( IS_DOWN_TRIANGLE(PSX_PAD1) )
    {
        notch.n4 = 1;
    } else {
        notch.n4 = 0;
    }

    //---- ブレーキ ----
    // ブレーキ1
    if ( IS_DOWN_R1(PSX_PAD1) ) {
        bra.b1 = 1;
    } else {
        bra.b1 = 0;
    }

    // ブレーキ2
    if ( IS_DOWN_L1(PSX_PAD1) ) {
        bra.b2 = 1;
    } else {
        bra.b2 = 0;
    }

    // ブレーキ3
    if ( IS_DOWN_R2(PSX_PAD1) ) {
        bra.b3 = 1;
    } else {
        bra.b3 = 0;
    }

    // ブレーキ4
    if ( IS_DOWN_L2(PSX_PAD1) ) {
        bra.b4 = 1;
    } else {
        bra.b4 = 0;
    }

    // ------------ ボタンによる音の再生要求 ------------
    // スタートボタン
    if ( IS_DOWN_START(PSX_PAD1) ) {
        // -- 警笛の再生
        if (hornFlag == false) {
            se[0] = PLAY;                            // 音再生の要求
            hornFlag = true;                         // 多重送信オフ
        }
    } else {
        se[0] = STOP;
        hornFlag = false;                            // 警笛フラグのオフ
    }

    // セレクト
    if ( IS_DOWN_SELECT(PSX_PAD1) ) {
        // -- ブザー音の再生
        if (buzzerFlag == false) {
            se[1] = PLAY;                            // 音再生の要求
            buzzerFlag = true;                       // 多重送信オフ
        }
    } else {
        se[1] = STOP;
        buzzerFlag = false;                          // フラグオフ
    }

    // Aボタン
    if ( IS_DOWN_SQUARE(PSX_PAD1) ) {
        // ドア開ける
        if (doorOpenFlag == false) {
            se[2] = PLAY;                            // 音再生の要求
            doorOpenFlag = true;                     // 多重送信オフ
        }
    } else {
        se[2] = STOP;
        doorOpenFlag = false;                        // フラグオフ
    }

    // Bボタン
    if ( IS_DOWN_CROSS(PSX_PAD1) ) {
        // ドア閉める音再生
        if (doorCloseFlag == false) {
            se[3] = PLAY;                            // 音再生の要求
            doorCloseFlag = true;                    // 多重送信オフ
        }
    } else {
        se[3] = STOP;
        doorCloseFlag = false;                       // フラグオフ
    }
    // Cボタン
    if ( IS_DOWN_CIRCLE(PSX_PAD1) ) {
        // 発車メロディ
        if (departureMusicFlag == false) {
            se[4] = PLAY;                            // 音再生の要求
            departureMusicFlag = true;               // 多重送信オフ
        }
    } else {
        se[4] = STOP;                                // フラグオフ
        departureMusicFlag = false;
    }

    pos = handle_pos( notch, bra );                  // マスコンとブレーキの構造体渡し
    return pos;                                      // ハンドルポジションの返却
}

//====================================================================
// ハンドル位置の返却
//====================================================================
Position handle_pos( struct Notch n, struct Brake b )
{
    // 中途半端なハンドルポジションを取得させないためにstaticを用いる
    static Position pos;
    // ブレーキが解除のときのみ、マスコンの処理に入る
    if ( b.b1 == 1 && b.b2 == 0 && b.b3 == 1 && b.b4 == 1 ) {
        // ノッチOFF
        if ( n.n1 == 1 && n.n2 == 1 && n.n3 == 1 && n.n4 == 0 ) {
            pos = OFF;
        }
        // 1ノッチ
        else if ( n.n1 == 0 && n.n2 == 1 && n.n3 == 1 && n.n4 == 1 ) {
            pos = N1;
        }
        // 2ノッチ
        else if ( n.n1 == 0 && n.n2 == 1 && n.n3 == 1 && n.n4 == 0 ) {
            pos = N2;
        }
        // 3ノッチ
        else if ( n.n1 == 1 && n.n2 == 1 && n.n3 == 0 && n.n4 == 1 ) {
            pos = N3;
        }
        // 4ノッチ
        else if ( n.n1 == 1 && n.n2 == 1 && n.n3 == 0 && n.n4 == 0 ) {
            pos = N4;
        }
        // 5ノッチ
        else if ( n.n1 == 0 && n.n2 == 1 && n.n3 == 0 && n.n4 == 1 ) {
            pos = N5;
        }
    } else {   // ブレーキが掛けられていたらこの処理に入る
        // ブレーキ1
        if ( b.b1 == 1 && b.b2 == 1 && b.b3 == 1 && b.b4 == 0 ) {
            pos = Br1;
        }
        // ブレーキ2
        else if ( b.b1 == 1 && b.b2 == 0 && b.b3 == 1 && b.b4 == 0 ) {
            pos = B2;
        }
        // ブレーキ3
        else if ( b.b1 == 0 && b.b2 == 1 && b.b3 == 1 && b.b4 == 1 ) {
            pos = B3;
        }
        // ブレーキ4
        else if ( b.b1 == 0 && b.b2 == 0 && b.b3 == 1 && b.b4 == 1 ) {
            pos = B4;
        }
        // ブレーキ5
        else if ( b.b1 == 0 && b.b2 == 1 && b.b3 == 1 && b.b4 == 0 ) {
            pos = B5;
        }
        // ブレーキ6
        else if ( b.b1 == 0 && b.b2 == 0 && b.b3 == 1 && b.b4 == 0 ) {
            pos = B6;
        }
        // ブレーキ7
        else if ( b.b1 == 1 && b.b2 == 1 && b.b3 == 0 && b.b4 == 1 ) {
            pos = B7;
        }
        // ブレーキ8
        else if ( b.b1 == 1 && b.b2 == 0 && b.b3 == 0 && b.b4 == 1 ) {
            pos = B8;
        }
        // 非常ブレーキ
        else if ( b.b1 == 0 && b.b2 == 0 && b.b3 == 0 && b.b4 == 0 ) {
            pos = EB;
        }
    }
    // ---- マスコン操作音の再生判定 ----
    if (pos != tmpPos) {                             // マスコンの位置が変わった
        se[5] = PLAY;                            // マスコン操作音再生  

    } else if (pos == tmpPos) {                      // マスコンの位置が変わらない
        se[5] = STOP;                                // マスコン操作音を再生しない
    }

  // ---- ブレーキ緩解音の再生判定 ----
    if (tmpPos==B6 || tmpPos==B5 || tmpPos==B4 || tmpPos==B3 && tmpPos==B2 || tmpPos==Br1) {
        if (pos==OFF || pos==N1 || pos==N2 || pos==N3 || pos==N4 || pos==N5) {
            se[6] = PLAY;   
        } else {
            se[6] = STOP;
        }
    } else {
        se[6] = STOP;
    }

    tmpPos = pos; 
    
    return pos;                                                           // ハンドル位置の返却
}

//====================================================================
//  ハンドル位置に応じてスピードの返却
//====================================================================
double return_speedVal( Position pos, double speed )
{
    // ハンドルの位置に応じたスピードの返却
    switch ( pos ) {
        // N5のとき
        case N5:
            speed += 0.5;
            break;

        // N4のとき
        case N4:
            speed += 0.4;
            break;

        // N3のとき
        case N3:
            speed += 0.3;
            break;

        // N2のとき
        case N2:
            speed += 0.2;
            break;

        // N1のとき
        case N1:
            speed += 0.1;
            break;

        // OFFのとき
        case OFF:
            speed += 0.0;
            break;

        // Br1のとき
        case Br1:
            speed -= 0.1;
            break;

        // B2のとき
        case B2:
            speed -= 0.2;
            break;

        // B3のとき
        case B3:
            speed -= 0.3;
            break;

        // B4のとき
        case B4:
            speed -= 0.4;
            break;

        // B5のとき
        case B5:
            speed -= 0.5;
            break;

        // B6のとき
        case B6:
            speed -= 0.6;
            break;

        // B7のとき
        case B7:
            speed -= 0.7;
            break;

        // B8のとき
        case B8:
            speed -= 0.8;
            break;

        // EBのとき
        case EB:
            speed -= 10.0;
            break;
    }

    speed = constrain( speed, 0.0, 128.0 );          // pwm出力は0-255の間なのでconstrainで範囲を制限する
    return speed;                                    // スピードの返却
}
