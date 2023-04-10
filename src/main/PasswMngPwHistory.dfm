object PasswHistoryDlg: TPasswHistoryDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Password history'
  ClientHeight = 191
  ClientWidth = 298
  Color = clBtnFace
  Constraints.MinHeight = 200
  Constraints.MinWidth = 314
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  DesignSize = (
    298
    191)
  PixelsPerInch = 96
  TextHeight = 13
  object EnableHistoryCheck: TCheckBox
    Left = 8
    Top = 8
    Width = 214
    Height = 17
    Anchors = [akLeft, akTop, akRight]
    Caption = 'Keep last passwords - up to:'
    TabOrder = 0
    OnClick = EnableHistoryCheckClick
  end
  object HistorySizeBox: TEdit
    Left = 232
    Top = 8
    Width = 41
    Height = 21
    Anchors = [akTop, akRight]
    TabOrder = 1
    Text = '1'
  end
  object HistorySizeSpinBtn: TUpDown
    Left = 273
    Top = 8
    Width = 16
    Height = 21
    Anchors = [akTop, akRight]
    Associate = HistorySizeBox
    Min = 1
    Position = 1
    TabOrder = 2
  end
  object HistoryView: TListView
    Left = 8
    Top = 35
    Width = 281
    Height = 118
    Anchors = [akLeft, akTop, akRight, akBottom]
    Columns = <
      item
        Caption = 'Set Date/Time'
      end
      item
        Caption = 'Password'
      end>
    MultiSelect = True
    ReadOnly = True
    RowSelect = True
    TabOrder = 3
    ViewStyle = vsReport
    OnKeyDown = HistoryViewKeyDown
  end
  object ClearBtn: TButton
    Left = 8
    Top = 159
    Width = 57
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Clear'
    TabOrder = 4
    OnClick = ClearBtnClick
  end
  object CopyBtn: TButton
    Left = 71
    Top = 159
    Width = 58
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Copy'
    TabOrder = 5
    OnClick = CopyBtnClick
  end
  object OKBtn: TButton
    Left = 135
    Top = 159
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 6
  end
  object CancelBtn: TButton
    Left = 216
    Top = 159
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 7
  end
end
