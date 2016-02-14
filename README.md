# [WIP] VGS Programable Sound Generator (decoder)

## About
現時点のVGSが搭載している音源システムは波形メモリ音源だが、当初はPSGで設計しようとしていた。
せっかくなのでPSG版VGS音源も作ってみる。

## Status: prototype
#### 2014.2.14
- `vgs-spu` を使って矩形波を鳴らす
- key-on/key-off をコマンド(on/off)で実装できるようにする
- エンベロープを実装
  - 従来のVGSはAT/RTのみ
  - AT/DT/ST/RTを実装してみる
- 周波数について
  - `vgs-spu` のリポジトリで思いっきり嘘をこいていたが、サンプリング周波数22050Hzで440で割ると約55Hzですね（ラじゃなかった）
  - コマンドで幾つで割るかを指定できるようにした
  - サンプリング周波数を44100Hzにしてみた（その方が細かい分割ができるので）
  - ビットレートを8bitにしてみた（計算し易い＆ファミコンも8bitなのでそれで十分だろうと）
  - _確かファミコンのサンプリングレートは32kHzだった（と思う）ので、ファミコンよりは音程が正確にできそう_
