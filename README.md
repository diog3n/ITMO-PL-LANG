# Blaise
Интерпретируемый/компилируемый язык высокоуровневый язык программирования, вдохновленный языком программмирования Pascal.
## Пример
Пример программы на данном языке (файл test.bls).

```Blaise
c = 4.0;
a = -2;
b = 3;
boo = c == 4;

d = (a + b) * c / 10;

function func1(a, b)
begin
    a = 5;
    b = 3;

    return a / b;
end

function func2(a, b)
begin
    writeln("a = " + a);
    writeln("b = " + b);
end

function func3() writeln("nothing");

function func4() return true;


if (boo == true) then writeln("yet again");

writeln("boo = " + boo);
writeln("a = " + a);
if (c >= 4) then
begin
    writeln("inside of the if block!");
    d = c;

    writeln("d = " + d);
end
else if (a < 0) then
begin
    writeln("inside of the else if block!");
end
else if ((a + b) > 2) then
begin
    writeln("oh no");
end
else
begin
    writeln("inside of the else block!");
end

if (c < 2) then writeln("inside if expression!");
else if (a < 0) then writeln("inside else if expression");
else writeln("inside else expression!");

i = 0;

loop if (i < 10) begin
    writeln("i = " + i);
    i = i + 1;
end

writeln("func1(a, b) = " + func1(a + c + d, b));
func2(a, b);

writeln(a);
writeln(b);
writeln(c);

writeln("(2 + c * 3) / 10 = " + (2 + c * 3) / 10);

writeln("Hello world!");

func3();

writeln("func4() = " + func4());
```
## Интерпретация
Интерпретировать код можно с помощью команды:
```bash
blaise interp [input_file.bls]
```
При прямой интерпретации вывод программы, описанной выше, будет таким:
```
yet again
boo = true
a = -2
inside of the if block!
d = 4
inside else if expression
i = 0
i = 1
i = 2
i = 3
i = 4
i = 5
i = 6
i = 7
i = 8
i = 9
func1(a, b) = 1
a = -2
b = 3
-2
3
4
(2 + c * 3) / 10 = 1.4
Hello world!
nothing
func4() = true
```
## Компиляция в трехадресный код
Помимо прямой интерпретации поддерживается также компиляция в трехадресный код:
```bash
blaise comp [input_file.bls]
```
Вывод для программы test.bls будет следующим:
```
c = 4.0
__BlaiseCompilerTmp_t0 = -2
a = __BlaiseCompilerTmp_t0
b = 3
__BlaiseCompilerTmp_t1 = c == 4
boo = __BlaiseCompilerTmp_t1
__BlaiseCompilerTmp_t2 = a + b
__BlaiseCompilerTmp_t3 = c / 10
__BlaiseCompilerTmp_t4 = __BlaiseCompilerTmp_t2 * __BlaiseCompilerTmp_t3
d = __BlaiseCompilerTmp_t4
function func1(a, b)begin
a = 5
b = 3
__BlaiseCompilerTmp_t5 = a / b
return __BlaiseCompilerTmp_t5
end
function func2(a, b)begin
__BlaiseCompilerTmp_t6 = "a = " + a
writeln(__BlaiseCompilerTmp_t6)
__BlaiseCompilerTmp_t7 = "b = " + b
writeln(__BlaiseCompilerTmp_t7)
end
function func3()writeln("nothing")
function func4()return true
__BlaiseCompilerTmp_t8 = boo == true
if (__BlaiseCompilerTmp_t8) then writeln("yet again")

__BlaiseCompilerTmp_t9 = "boo = " + boo
writeln(__BlaiseCompilerTmp_t9)
__BlaiseCompilerTmp_t10 = "a = " + a
writeln(__BlaiseCompilerTmp_t10)
__BlaiseCompilerTmp_t11 = c >= 4
__BlaiseCompilerTmp_t13 = a < 0
__BlaiseCompilerTmp_t14 = a + b
__BlaiseCompilerTmp_t15 = __BlaiseCompilerTmp_t14 > 2
if (__BlaiseCompilerTmp_t11) then begin
writeln("inside of the if block!")
d = c
__BlaiseCompilerTmp_t12 = "d = " + d
writeln(__BlaiseCompilerTmp_t12)
end
else if (__BlaiseCompilerTmp_t13) then begin
writeln("inside of the else if block!")
end
else if (__BlaiseCompilerTmp_t15) then begin
writeln("oh no")
end
else begin
writeln("inside of the else block!")
end
__BlaiseCompilerTmp_t16 = c < 2
__BlaiseCompilerTmp_t17 = a < 0
if (__BlaiseCompilerTmp_t16) then writeln("inside if expression!")
else if (__BlaiseCompilerTmp_t17) then writeln("inside else if expression")
else writeln("inside else expression!")
i = 0
loop if (i<10) begin
__BlaiseCompilerTmp_t18 = "i = " + i
writeln(__BlaiseCompilerTmp_t18)
__BlaiseCompilerTmp_t19 = i + 1
i = __BlaiseCompilerTmp_t19
end
__BlaiseCompilerTmp_t20 = c + d
__BlaiseCompilerTmp_t21 = a + __BlaiseCompilerTmp_t20
__BlaiseCompilerTmp_t22 = "func1(a, b) = " + func1(__BlaiseCompilerTmp_t21, b)
writeln(__BlaiseCompilerTmp_t22)
func2(a, b)
writeln(a)
writeln(b)
writeln(c)
__BlaiseCompilerTmp_t23 = c * 3
__BlaiseCompilerTmp_t24 = 2 + __BlaiseCompilerTmp_t23
__BlaiseCompilerTmp_t25 = __BlaiseCompilerTmp_t24 / 10
__BlaiseCompilerTmp_t26 = "(2 + c * 3) / 10 = " + __BlaiseCompilerTmp_t25
writeln(__BlaiseCompilerTmp_t26)
writeln("Hello world!")
func3()
__BlaiseCompilerTmp_t27 = "func4() = " + func4()
writeln(__BlaiseCompilerTmp_t27)
```
