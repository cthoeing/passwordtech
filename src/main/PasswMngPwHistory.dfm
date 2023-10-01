object PasswHistoryDlg: TPasswHistoryDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Password history'
  ClientHeight = 239
  ClientWidth = 381
  Color = clBtnFace
  Constraints.MinHeight = 229
  Constraints.MinWidth = 316
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnShow = FormShow
  PixelsPerInch = 120
  DesignSize = (
    381
    239)
  TextHeight = 17
  object EnableHistoryCheck: TCheckBox
    Left = 13
    Top = 11
    Width = 233
    Height = 21
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akTop, akRight]
    Caption = 'Keep last passwords - up to:'
    TabOrder = 0
    OnClick = EnableHistoryCheckClick
    ExplicitWidth = 227
  end
  object HistorySizeBox: TEdit
    Left = 292
    Top = 9
    Width = 51
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akTop, akRight]
    TabOrder = 1
    Text = '1'
  end
  object HistorySizeSpinBtn: TUpDown
    Left = 346
    Top = 11
    Width = 20
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akTop, akRight]
    Associate = HistorySizeBox
    Min = 1
    Position = 1
    TabOrder = 2
  end
  object HistoryView: TListView
    Left = 13
    Top = 44
    Width = 353
    Height = 140
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akTop, akRight, akBottom]
    Columns = <
      item
        Caption = 'Set Date/Time'
        Width = 63
      end
      item
        Caption = 'Password'
        Width = 63
      end>
    MultiSelect = True
    ReadOnly = True
    RowSelect = True
    TabOrder = 3
    ViewStyle = vsReport
    OnKeyDown = HistoryViewKeyDown
    ExplicitWidth = 347
    ExplicitHeight = 139
  end
  object ClearBtn: TButton
    Left = 13
    Top = 196
    Width = 71
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akBottom]
    Caption = 'Clear'
    TabOrder = 4
    OnClick = ClearBtnClick
    ExplicitTop = 195
  end
  object CopyBtn: TButton
    Left = 88
    Top = 196
    Width = 71
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akBottom]
    Caption = 'Copy'
    TabOrder = 5
    OnClick = CopyBtnClick
    ExplicitTop = 195
  end
  object OKBtn: TButton
    Left = 174
    Top = 196
    Width = 92
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 6
    ExplicitLeft = 168
    ExplicitTop = 195
  end
  object CancelBtn: TButton
    Left = 274
    Top = 196
    Width = 92
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 7
    ExplicitLeft = 268
    ExplicitTop = 195
  end
end
