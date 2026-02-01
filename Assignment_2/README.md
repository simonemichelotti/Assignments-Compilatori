# Secondo Assignment

---

## 1) Very Busy Expressions
Un’espressione a+b è very busy in
un punto p se a+b è valutata in
tutti i percorsi da p a EXIT e non
c’è una definizione di a o b lungo
tali percorsi

### Formalization Table
||Very Busy Expressions
---|:---:
Domain| Sets of Expressions
Direction| Backward: <br> $in[b]=f_b(out[b])$ <br> $out[b]=\land \ in[succ(b)]$
Tranfer Function| $f_b(x)=Gen_b \ \cup \ (x-Kill_b) $
Meet Operation ($\land$)| $\cap$
Boundary Condition| $in[exit] = \emptyset$
Initial Interior Points| $in[b] = U$

|      | **1** |        | **2** |        |
|------|-------|--------|-------|--------|
|      | **IN** | **OUT** | **IN** | **OUT** |
| Exit | Ø     |        | Ø     | Ø      |
| BB4  | U     |        | (a-b) | Ø      |
| BB3  | U     |        | (a-b), (b-a) | (a-b) |
| BB7  | U     |        | (a-b) | Ø      |
| BB6  | U     |        | Ø     | (a-b) |
| BB5  | U     |        | (b-a) | Ø      |
| BB2  | U     |        | **(b-a)** | {(a-b), (b-a)} ∩ {(b-a)} = (b-a) |

La Very Busy Expression nel punto p è "b-a"

---

## 2) Dominator Analysis
In un CFG diciamo che un
nodo X domina un altro
nodo Y se il nodo X
appare in ogni percorso
del grafo che porta dal
blocco ENTRY al blocco Y

### Formalization Table
||Dominator Analysis
---|:---:
Domain| Sets of BasicBlocks
Direction| Forward: <br> $out[b]=f_b(in[b])$ <br> $in[b]=\land \ out[pred(b)]$
Tranfer Function| $f_b(x)=Gen_b \ \cup \ x $
Meet Operation ($\land$)| $\cap$
Boundary Condition| $out[entry] = entry$
Initial Interior Points| $out[b] = U$

| Entry | 1 IN | 1 OUT | 2 IN                | 2 OUT      |
|------:|:----:|:-----:|:--------------------|:-----------|
| Entry |      | ∅     | ∅                   | ∅          |
| A     |      | U     | ∅                   | A          |
| B     |      | U     | A                   | A, B       |
| C     |      | U     | A                   | A, C       |
| D     |      | U     | A, C                | A, C, D    |
| E     |      | U     | A, C                | A, C, E    |
| F     |      | U     | out(D) ∩ out(E) = A, C | A, C, F |
| G     |      | U     | out(B) ∩ out(F) = A | A, G       |

---

## 3) Constant Propagation
L’obiettivo della constant propagation è
quello di determinare in quali punti del
programma le variabili hanno un valore
costante.
L’informazione da calcolare per ogni
nodo n del CFG è un insieme di coppie
del tipo <variabile, valore costante>.
Se abbiamo la coppia <x, c> al nodo n,
significa che x è garantito avere il valore
c ogni volta che n viene raggiunto
durante l’esecuzione del programma.

### Formalization Table
||Constant Propagation
---|:---:
Domain| Sets of (\<variable>,\<const-value>)
Direction| Forward: <br> $out[b]=f_b(in[b])$ <br> $in[b]=\land \ out[pred(b)]$
Tranfer Function| $f_b(x)=Gen_b \ \cup \ (x-Kill_b)$
Meet Operation ($\land$)| $\cap$
Boundary Condition| $out[entry] = \emptyset$
Initial Interior Points| $out[b] = U$

| Riga   | 0 IN | 0 OUT | 1 IN | 1 OUT | 2 IN | 2 OUT | 3 IN | 3 OUT |
|--------|------|-------|------|-------|------|-------|------|-------|
| entry  | ∅    | ∅     | ∅    | ∅     |      |       |      |       |
| k=2    |      | U     | (k,2) | (k,2) |      |       |      |       |
| if     |      | U     | (k,2) | (k,2) |      |       |      |       |
| a=k+2  |      | U     | (k,2) | (k,2), (a,4) | | | | |
| x=5    |      | U     | (k,2), (a,4) | (k,2), (a,4), (x,5) | | | | |
| a=k*2  |      | U     | (k,2) | (k,2), (a,4) | | | | |
| x=8    |      | U     | (k,2), (a,4) | (k,2), (a,4), (x,8) | | | | |
| k=a    |      | U     | (k,2),(a,4),(x,5) ∩ (k,2),(a,4),(x,8) = (k,2),(a,4) | (k,4),(a,4) | | | | |
| while  |      | U     | (k,4),(a,4) | (k,4),(a,4) | (k,5),(a,4),(b,2),(x,8),(y,a*b) ∩ (k,4),(a,4) = (a,4) | (a,4) | (a,4),(b,2),(y,a*b) ∩ (k,4),(a,4) = (a,4) | (a,4) |
| b=2    |      | U     | (k,4),(a,4) | (k,4),(a,4),(b,2) | (a,4) | (a,4),(b,2) | (a,4) | (a,4),(b,2) |
| x=a+k  |      | U     | (k,4),(a,4),(b,2) | (k,4),(a,4),(b,2),(x,a+k) | (a,4),(b,2) | (a,4),(b,2) | (a,4),(b,2) | (a,4),(b,2) |
| y=a*b  |      | U     | (k,4),(a,4),(b,2),(x,a+k) | (k,4),(a,4),(b,2),(x,8),(y,a*b) | (a,4),(b,2) | (a,4),(b,2),(y,a*b) | (a,4),(b,2) | (a,4),(b,2),(y,a*b) |
| k=k+1  |      | U     | (k,4),(a,4),(b,2),(x,8),(y,a*b) | (k,5),(a,4),(b,2),(x,8),(y,a*b) | (a,4),(b,2),(y,a*b) | (a,4),(b,2),(y,a*b) | (a,4),(b,2),(y,a*b) | (a,4),(b,2),(y,a*b) |
| print  |      | U     | out(while) = (a,4) | (a,4) | | | | |
| exit   |      | U     | (a,4) | (a,4) | | | | |


