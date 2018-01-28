Program Srec2asc;
{ SREC2ASC - Convert Motorola S-Record file to          }
{            POM 1 ASCII File                           }
{ (C) 2017 - Curtis F Kaylor                            }

Uses
  SysUtils, StrUtils;

Const
  SrecExt = '.srec';
  AscExt = '.asc';

Var
  SrecFile: TextFile; {Input File}
  AscFile: TextFile;  {Output File}

Procedure OpenSrecFile;
Var
  LastDot, LastSlash: Integer;
  OpenResult: Integer;
Begin
  SrecFileName := ParamStr(1);
  LastDot := RPos('.', SrecFileName);
  LastSlash := RPos('/', SrecFileName);
  If LastDot <= LastSlash Then
    SrecFileName := SrecFileName + SrecExt;
  If Debug Then WriteLn('Opening File ', SrecFileName);
  Assign(SrecFile, SrecFileName);
  {$I-}
  Reset(SrecFile);
  OpenResult := IOResult;
  {$I+}
  If OpenResult <> 0 Then
    Error('Error Opening Input File ' + SrecFileName);
End;


Begin
  


End;
