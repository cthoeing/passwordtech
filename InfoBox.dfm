object InfoBoxForm: TInfoBoxForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsToolWindow
  ClientHeight = 28
  ClientWidth = 266
  Color = clInfoBk
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object TextLbl: TLabel
    Left = 8
    Top = 8
    Width = 35
    Height = 13
    Caption = 'TextLbl'
  end
  object Timer: TTimer
    OnTimer = TimerTimer
    Left = 120
  end
end
