; 16-digit BCD addition routine
  JUN START
  NOP
  BBS
START
  JMS BCDADD
DONE
  JUN DONE
  
BCDADD
  FIM P0, 0
  FIM P1, 48
  LDM 0
  XCH R6
  CLC
ADD1
  SRC P1
  RDM
  SRC P0
  ADM
  DAA
  WRM
  INC R1
  INC R5
  ISZ R6, ADD1
OVERFL
  JCN %0010, XXX
  JUN NEXT
XXX
  LDM 0
  XCH RA
  
  BBL
