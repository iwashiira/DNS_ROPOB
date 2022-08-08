DNS_ROPOB
===============

# 概要
* [ROPOB: Obfuscating Binary Code
via Return Oriented Programming](https://mudongliang.github.io/files/papers/ropob_securecomm.pdf)という論文を参考に、TSG LIVE CTF 8のために作ったプログラム。汚いコードなのは許してほしい。(時間があれば綺麗にする。)
* `input/dns_ropob.c`が問題のソースコード
* どのような仕組みかは、変換後のバイナリをstep実行すれば自ずとわかる。
* 論文と同じく、動的解析には無力

# 論文との相違点。

* 論文ではバイナリに対して変更を施していたが、.sファイルに対して変更を施すようにした。
  * 実装を確認できないので、アセンブリを編集した可能性はある。
  * これにより、相対アドレッシングのアドレス解決が簡単になった。
* 論文ではガジェットからガジェットへとオフセットテーブルを参照しながらretを介して飛んでいくようにしたようだが、リゾルバ関数を作ってそこを経由するようにした。
  * 次のガジェットの前に別の関数を迂回するということで、すべてのレジスタの値が完全に保存されている状態で次のガジェットへとたどり着く必要がある。
  * pushfqを二回行ってreturnアドレスの格納用の領域を作ったのは多分オリジナル?
* 論文では多分機械語の長さの取得は独自のプログラムでやったのだろうが、面倒だったのでobjdumpを使って読み取るようにした。
  * ガジェット化に伴ってJMP命令のアドレッシングの距離が変化してしまうが故に、ソースコードから作られた.sファイルからアセンブルリンクされてできるexecutableと、ガジェット化が施された.sファイルからアセンブルリンクされてできるexecutableの機械語長が一致しない問題は、JMP命令の長さの計算だけ正しくない.sファイルを作ってアセンブルリンクして作り出したexecutableからさらにobjdumpすることで、正しいJMP命令の長さを計算するというかなりのゴリ押しで解決した。
 
# 制約

* x86_64のELFバイナリのみ作り出せる。
* objdumpとgccに依存している。

# 使い方

```
make
./dns_ropob_generator hogehoge.c
```
# 以下細かい説明

## ガジェットからガジェットへの遷移の実現のための課題。
1. ガジェットからガジェットへの行き来のためには次に到達すべきガジェットのアドレスを知る必要がある。
2. 特定の命令がちゃんと動く必要がある。push pop leave call jmp cmp ret等

**方針**
* functionに番号をふり、その中のガジェットにも番号をふる。
* JMP系命令 => そのまま実行
* それ以外 => resolver()を作ってそれに頼る。
  * resolver()を作るメリットとしては、ガジェットはresolver()にretして移動するだけでよく、resolver()はガジェットのオフセットテーブルを適切に作ることができれば、それを見て次のガジェットに簡単にretできる。つまり、ガジェット自身の中で次のガジェットへと移動するための複雑な機構を設ける必要はなく、最低限のfunctionの番号とその中のガジェット自身の番号さえresolverに渡せれば良い。今回はそれに加えfunctionのbaseアドレスも渡すようにした。
    - retを使うので、(工夫すればできるけれど)stack渡しはしない。グローバル変数等を使う。
* 特定の命令が動くためには、各ガジェットの中の細分化する前の元の命令を実行する際に、stack、rflagsを含めたすべてのレジスタを元の.sでその命令を実行する際の状態と完全に一致させればよい。

**各ガジェットとresolver()のコードは以下のようにすれば、方針通りに動作をする。**

* gadget()
```func.s
[original instruction]
pushfq
pushfq
push	rax
lea	rax, func[rip]
mov	QWORD PTR base[rip], rax
mov	QWORD PTR gadgetnumber[rip], 0
mov	QWORD PTR funcnumber[rip], 0
lea	rax, resolver[rip]
mov	QWORD PTR [rsp+16], rax
pop	rax
popfq
ret
```

* resolver()
```resolver.s
pushfq
pushfq
push	rax
push	rcx
push	rdx
mov	rax, QWORD PTR funcnumber[rip]
lea	rcx, funcgadgetoffsets[rip]
mov	rdx, QWORD PTR [rcx+rax*8]
mov	rax, rdx
mov	rcx, QWORD PTR gadgetnumber[rip]
add	rcx, 1
mov	rdx, QWORD PTR [rax+rcx*8]
mov	rax, QWORD PTR base[rip]
add	rax, rdx
mov	QWORD PTR [rsp+32], rax
pop	rdx
pop	rcx
pop	rax
popfq
ret
```

* registerの値を変えずにリターンアドレスを設定するには、pushfqを2回行ってreturnアドレス用の領域を作り、適宜操作に必要なregisterの値をpushして保存し、終わり際にmov命令でreturnアドレスの部分だけresolver()のアドレスを入れた状態でpopでregisterの値を復元すると、retを踏んで次のガジェットに移った時にはstackとregisterの値は`original instruction`を実行したときと全く同じになる。
  * 特定の命令が正しく実行できる。rflagsも保存しないと、cmp命令等が正しく機能しない。sub命令ではリターンアドレス用の領域は作れない。
* 各ガジェットはfuncのシンボルのアドレスとgadgetのnumberとfunctionのnumberをそれぞれ、グローバル変数のbase, gadgetnumber, funcnumberに格納する。
* resolver()はfuncnumberとgadgetnumberを使ってfuncgadgetoffsetsからfunc{funcnumber}gadgettableのアドレスを取り出し、そのテーブルから次のガジェットのオフセットを取り出してbaseにプラスし、こうして得られた次のガジェットのアドレスをreturnアドレスに格納する。
  * func{funcnumber}gadgettableはガジェットのナンバーの順序に乗っ取って整列していないといけない。(具体的にはfunc0gadgettable\[5\]は0から始まるgadgetnumberの5番に相当するガジェット、つまり6個目のoriginal instructionのfuncnumber=0のfunction内オフセットを格納していないといけない)
  * resolver()が各funcgadgetoffsetsからfuncgadgettableのアドレスを取り出せるように、main関数の先頭において、mainのoriginal instructionを実行する前にfuncgadgetoffsetsの中をfunctionの数分適宜アドレスを入れて初期化する。

## 自動化の流れ。
把握していく順番であって、記入していく順番は出来上がるアセンブリがコンパイルできるように、必要な順に記入していく。

**0. Cのコードを受け取りアセンブリのコードを作り、それを編集したりコピーしたりする。**

**1. functionの数を特定する。**
* funcgadgetoffsetsに必要なサイズを特定し作成できる。
* global変数のサイズは基本64bit(.quad)とした

**2. 各functionの命令の数をカウント**
* 各functionのfuncgadgettableのサイズを特定。
  * まだこの時点では実際の各命令のオフセットはわからない。

**3.　各functionの各命令の機械語長を調べる。**
* originalの.cから作ったoriginalの.sファイルからexecutableを作り、それに対してobjdump -dした結果をobjファイルとして作成しそこから各機械語長を読みだした。
  * executableを作る際にはgccにオプション`-masm=intel -fno-asynchronous-unwind-tables -mno-red-zone`を渡した。特に、`-mno-red-zone`がないと、自身の中でサブルーチンを呼ばないルーチンなどで、rspを減算してスタックを伸長させることなく直でrbp-numberで操作をしてしまうことがあり、その場合original instruction後のpush命令がそのローカル変数を書き換えてしまう。
* objdumpの結果は一部長い機械語の行を折り返してしまったり、各行の先頭のスペースの個数が違ったりするのでパースするには最新の注意を払うこと。
* JMP系命令だけは、まだ本当の機械語長を知ることはできない。ガジェット化やランダム化をした際に、飛び先のアドレスまでの距離が変化し、アドレッシングが変わってしまう場合が存在するためである。その場合、original instructionそのもののアセンブリの表記は同じでも、機械語長は一致しないことがある。

**4. function内で各命令の順番をランダム化する。**
* ガジェット配列にランダム化後のgadgetnumberの順序を格納する。
* 元の命令の順番通りに実行する必要はあるのでoriginal instructionとgadgetnumberの組み合わせは変化しない。

**5. function内の各命令のオフセットを計算する。**
* ガジェット化した後のオフセットを計算する。
- 自身のオフセット = `ガジェット配列内における一つ前に配置されているガジェットのオフセット + 一つ前のガジェットの機械語長`
- 得られたオフセットをgadgetnumber順にfuncgadgettableとしてout1.sというアセンブリファイルに追加

**6. gadgetを実際に作成し、記入していく**
* Labelはoriginal instructionに飛んでいくための目印なので、ガジェットには分離しない。

**7. resolver()を記入**

**8. mainに初期化処理を記入**

**9. 出来上がった.out1.sをコンパイル**
* JMP系命令の機械語長だけを間違えているexecutableが出来上がる。(当然オフセットも間違っている。)
* JMP系命令の実際の位置は定まったのでこの状態でそのexecutableに対し再度objdumpを適用し、JMP系の命令だけ機械語長をgadgetnumberと共に取得
* 取得した機械語長の分、funcgadgettableのオフセットは計算し直す。

**10. out1.sのfuncgadgettableのオフセットだけ正しいものに直したout2.sをコピーを駆使して作りだし、コンパイルすると難読化されても正常に実行されるバイナリが得られる。**

上記のような推移をしていくコードを書いた結果がこのリポジトリのdns_ropob_generatorです。

## まとめ

* アセンブリファイルを編集しているので、original instructionそのものへの難読化との相性が良さそう。
* 静的解析はrandom化すると見違えるほどに難しくなる。動的解析への対応ができたらさらに強くなりそう。
* packerとかと組み合わせるとさらに難しくなりそう。
