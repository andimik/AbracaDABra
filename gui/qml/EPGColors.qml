/*
 * This file is part of the AbracaDABra project
 *
 * MIT License
 *
 * Copyright (c) 2019-2025 Petr Kopecký <xkejpi (at) gmail (dot) com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

pragma Singleton
import QtQuick

QtObject {    
    id: epgColors
    readonly property color bgColor:                    epgDialog.colors[0] // "black"

    readonly property color textColor:                  epgDialog.colors[1] // "black"
    readonly property color fadeTextColor:              epgDialog.colors[2] // "#606060"
    readonly property color gridColor:                  epgDialog.colors[3] // "lightgray"

    readonly property color highlightColor:             epgDialog.colors[4] //"#3e9bfc"
    readonly property color selectedBorderColor:        epgDialog.colors[5]

    readonly property color pastProgColor:              epgDialog.colors[6] // "#e4e4e4"
    readonly property color nextProgColor:              epgDialog.colors[7] // "#f9f9f9"
    readonly property color currentProgColor:           epgDialog.colors[8] // "#E2F4FF"
    readonly property color progressColor:              epgDialog.colors[9] // "#A4DEFF"

    readonly property color emptyLogoColor:             epgDialog.colors[10] // "white"
    readonly property color shadowColor:                epgDialog.colors[11] // "darkgray"

    readonly property color switchBgColor:              epgDialog.colors[12]
    readonly property color switchBorderColor:          epgDialog.colors[13]
    readonly property color switchHandleColor:          epgDialog.colors[14]
    readonly property color switchHandleBorderColor:    epgDialog.colors[15]
    readonly property color switchHandleDownColor:      epgDialog.colors[16]

    readonly property color buttonColor:                epgDialog.colors[17]
    readonly property color buttonDownColor:            epgDialog.colors[18]
    readonly property color buttonBorderColor:          epgDialog.colors[19]
}

