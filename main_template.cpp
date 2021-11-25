/*
 * Copyright (c) 2018-2020, sigboost, inc., All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of sigboost nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "midiglue_node.h"

/*
    If you want to define your own global functions, classes, and variables, write them here.

    NOTE:
    You are free to use this space, but we cannot support you if something goes wrong.
    Also, please understand that using this space will destroy the modularity of the node system.
    (Nodes that depend on your newly defined variables or something will lose portability.)
    We recommend that you add functionality by creating new nodes.
*/

/*********************************************
 * Launch Pad用定数 start 
**********************************************/
/** locate(0,Y)のsysEx番号 */
// 10進の方が解りやすいので10進で書いています
static const std::vector<uint8_t> LP3_yNo = {81,71,61,51,41,31,21,11,};
/** カラー */
static const uint8_t LP3C_BLACK = 0x00;
static const uint8_t LP3C_DK_GLAY = 0x01;
static const uint8_t LP3C_LGT_GLAY = 0x02;
static const uint8_t LP3C_WHITE = 0x03;
static const uint8_t LP3C_RED = 0x05;
static const uint8_t LP3C_D_RED = 0x07;
static const uint8_t LP3C_L_YELLOW = 0x0C;
static const uint8_t LP3C_YELLOW = 0x0D;
static const uint8_t LP3C_Y_GREEN = 0x11;
static const uint8_t LP3C_L_GREEN = 0x14;
static const uint8_t LP3C_GREEN = 0x15;
static const uint8_t LP3C_LGY_BLUE = 0x25;
static const uint8_t LP3C_GLY_BLUE = 0x27;
static const uint8_t LP3C_BLUE = 0x29;
static const uint8_t LP3C_G_BLUE = 0x2A;
static const uint8_t LP3C_D_BLUE = 0x2B;
static const uint8_t LP3C_BL_PURPLE = 0x2D;
static const uint8_t LP3C_PURPLE = 0x31;
static const uint8_t LP3C_PINK = 0x35;
static const uint8_t LP3C_ORANGE = 0x3C;
static const uint8_t LP3C_L_ORANGE = 0x3D;
/** モード */
static const uint8_t LP3B_NOMAL = 0;    // ノーマル
static const uint8_t LP3B_FLASH = 1;    // 点滅
static const uint8_t LP3B_PULS = 2;     // 明滅
/** X-Yの最大値 */
static const uint8_t LP3_maxXY = 8;
/** 特殊キー */
static const uint8_t LP3K_UP = 91;
static const uint8_t LP3K_DOWN = 92;
static const uint8_t LP3K_LEFT = 93;
static const uint8_t LP3K_RIGHT = 94;
static const uint8_t LP3K_SESSION = 95;
static const uint8_t LP3K_DRUMS = 96;
static const uint8_t LP3K_KEYS = 97;
static const uint8_t LP3K_USER = 98;
static const uint8_t LP3K_LOGO = 99;
/*********************************************
 * Launch Pad用定数 end 
**********************************************/

/*********************************************
 * MuteBox用定数 start
**********************************************/
static const uint8_t LP3_MB_MODE = 0x03;            // muteBox モード：モードマスク
static const uint8_t LP3_MB_MODE_PLAY = 0x01;       // muteBox モード；演奏中
static const uint8_t LP3_MB_MODE_EDIT_ST = 0x02;    // muteBox モード；編集モード開始
static const uint8_t LP3_MB_MODE_EDIT_ED = 0x03;    // muteBox モード；編集モード終了
static const uint8_t LP3_MB_REFRESH_MUTE = 0x40;    // MUTEBOX リフレッシュ
/*********************************************
 * MuteBox用定数 end 
**********************************************/


/*********************************************
 * Launch Pad光点設定用のメソッド start
**********************************************/
class Lp3 {

public:
    // SysExへ光点の追加
    static void addDot(IntList buff,uint8_t mode,uint8_t x,uint8_t y,uint8_t color){
        // 値の設定
        buff->push_back(mode);
        buff->push_back(LP3_yNo[y] + x);
        buff->push_back(color);
    }

    // SysExへ光点の追加(位置計算無し)
    static void addDotLoc(IntList buff,uint8_t mode,uint8_t loc,uint8_t color){
        // 値の設定
        buff->push_back(mode);
        buff->push_back(loc);
        buff->push_back(color);
    }

    /**
     * midiパケットが、Launchpadのどの位置かを返す。
     * 
     */
    static bool getLocation(MidiPacket midi,uint8_t* x,uint8_t* y){

        // ステータス
        uint8_t status = midi.getControlNumber();

        // 特殊キーの場合 TODO:ホントはcontains出来たほうがいいんだろうけど。
        if(status>=LP3K_UP && status<=LP3K_LOGO) {
            *x = 0;
            *y = 0;
            return true;
        }

        // 位置取得
        *y = 8-(status)/10;
        *x = (status)%10-1;

        return false;

    }
};
/*********************************************
 * Launch Pad光点設定用のメソッド end
**********************************************/




$functions

static void setup()
{
    Midiglue::set_patch_info("$patch_name", "$patch_info");

    Midiglue::setup();

    if (Midiglue::flow_ok())
    {
    Midiglue::FlowLock lock();
    // ------------------------------------ hardware setup ------------------------------------
$hw_setup


    // ------------------------------------ node setup ------------------------------------
$setup

    }
    else
    {
    Midiglue::add_jackinput_sysex(Midiglue::USB_MIDI_DEVICE, Midiglue::Driver::USB_MIDI_DEVICE, Midiglue::Driver::USB_MIDI_DEVICE);
    Midiglue::add_jackoutput_normal(Midiglue::USB_MIDI_DEVICE, Midiglue::Driver::USB_MIDI_DEVICE);
    }

    Midiglue::setup_gui();
    Midiglue::activate();
}

int main()
{
    Midiglue::init();
    setup();
    Midiglue::system_loop();
}
