Sub EscapeObject(Parent As Object, Obj As Object, X As Single, Y As Single)
    'ボタンが逃げる処理
    
    '左側
    If Obj.Left - 10 < X Then
        '上下のチェック
        If Obj.Top < Y Then
            If Obj.Top + Obj.Height > Y Then
                '最後のチェック、右側
                If X < Obj.Left Then
                    MoveObject Parent, Obj, 0, 10, 0, 0
                End If
            End If
        End If
    End If
    
    '右側
    If Obj.Left + Obj.Width + 10 > X Then
        '上下のチェック
        If Obj.Top < Y Then
            If Obj.Top + Obj.Height > Y Then
                '最後のチェック、右側
                If Obj.Left + Obj.Width < X Then
                    MoveObject Parent, Obj, 0, 0, 0, 10
                End If
            End If
        End If
    End If
    
    '上側
    If Obj.Top - 10 < Y Then
        '左右のチェック
        If Obj.Left < X Then
            If Obj.Left + Obj.Width > X Then
                '最後のチェック
                If Obj.Top + Obj.Height > Y Then
                    MoveObject Parent, Obj, -10, 0, 0, 0
                End If
            End If
        End If
    End If
    
    '下側
    If Obj.Top + Obj.Height + 10 > Y Then
        '左右のチェック
        If Obj.Left < X Then
            If Obj.Left + Obj.Width > X Then
                '最後のチェック
                If Obj.Top < Y Then
                    MoveObject Parent, Obj, 10, 0, 0, 0
                End If
            End If
        End If
    End If
    
End Sub

Sub MoveObject(Parent As Object, Obj As Object, Top As Integer, Right As Integer, Bottom As Integer, Left As Integer)
    If Obj.Top - Top < 0 Then
        Top = Top - Top
        Right = Right + 20
    End If
    If Obj.Left - Left < 0 Then
        Left = Left - Left
        Top = Top + 20
    End If
    If Obj.Left + Obj.Width + Right > Parent.Width Then
        Right = Right - Right
        Bottom = Bottom + 20
    End If
    If Obj.Top + Obj.Height + Bottom > Parent.Height Then
        Bottom = Bottom - Bottom
        Left = Left + 20
    End If

    Obj.Top = Obj.Top - Top
    Obj.Left = Obj.Left + Right
    Obj.Top = Obj.Top + Bottom
    Obj.Left = Obj.Left - Left
End Sub

Private Sub UserForm_Click()
    Set cmd = Controls.Add("Forms.CommandButton.1", "test", Visible)
    cmd.Caption = "This is test"
    cmd.Top = Rnd() * 300 + 1
    cmd.Left = Rnd() * 300 + 1
End Sub

Private Sub UserForm_MouseMove(ByVal Button As Integer, ByVal Shift As Integer, ByVal X As Single, ByVal Y As Single)
    Dim tmp As Variant
    
    EscapeObject Field, Box, X, Y
    EscapeObject Field, TextBox1, X, Y
    EscapeObject Field, ListBox1, X, Y
    EscapeObject Field, ScrollBar1, X, Y
    EscapeObject Field, SpinButton1, X, Y
    EscapeObject Field, OptionButton1, X, Y
End Sub
