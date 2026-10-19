# Mac版Moonlightの開発引き継ぎ

更新日：2026-10-19 JST。共有先は [usm916/moonlight-qt の custom/main](https://github.com/usm916/moonlight-qt/tree/custom/main)。Windows用のランタイム選択修正もこのブランチへ統合する。GitHubの既定ブランチがmasterでも、開発時はcustom/mainを指定する。

全体の手順・Mac版Sunshineの移植先・TANの状態は [Sunshine側のMac開発引き継ぎ](https://github.com/usm916/Sunshine/blob/custom/main/docs/custom/mac-development.md) を読む。

## 取得とビルド

```sh
mkdir -p "$HOME/Developer/moonlight-custom"
cd "$HOME/Developer/moonlight-custom"
git clone --branch custom/main --recurse-submodules https://github.com/usm916/moonlight-qt.git moonlightCustom
cd moonlightCustom
```

Xcode 15以降の初期設定を済ませ、Qt 6.11以降のmacOS SDKを用意する。Core/Network/Quick/QuickControls2/SVGが必要。HomebrewのQtを使う場合は次の例を使う。Qt SDKインストーラーならPATHをそのmacos/binへ置き換える。IntelとApple Siliconでアーキテクチャを混在させない。

```sh
brew install qt
export PATH="$(brew --prefix qt)/bin:$PATH"
QMAKE="$(command -v qmake6 || command -v qmake)"
"$QMAKE" -query QT_VERSION
python3 setup-deps.py
mkdir -p cmake-build-macos-release
cd cmake-build-macos-release
"$QMAKE" ../moonlight-qt.pro
make -j4 release
```

通常の出力は `cmake-build-macos-release/app/Moonlight.app`。この独自版をMacでビルド・接続した結果はまだない。失敗時はOS版、CPU、Xcode/Qtの版、コミットID、最初のエラーを記録する。公式の取得版の [README](../../README.md#building) と `app/app.pro` がビルド構成の基準。

## 通信互換性

共通ライブラリは `moonlight-common-c/moonlight-common-c`、固定コミットは `ae67fe009386d536569fecc34ba156d581309b54`。対応するWindowsホストは [usm916/Sunshine](https://github.com/usm916/Sunshine/tree/custom/main)。ホストの `docs/custom/compatibility.json` に引き継ぎ対象クライアントのコミットを記録する。

独自テキスト同期は [clipboard-v1.md](../../moonlight-common-c/moonlight-common-c/docs/custom/clipboard-v1.md) に従う。既定OFF、認証したホストの能力・方向を確認、UTF-8で32,755バイトまで、空のクリップボードや画像・ファイルは同期しない。初期購読時にクライアントの内容を無条件に送ってホストを上書きしない。Qt/SDLによる実装はあるが、Macでの動作確認は必要。

## テストとTANへの接続

- 共通Cテストは `moonlight-common-c/moonlight-common-c/tests/clipboard.c`。クライアント状態遷移テストは `tests/custom/clipboard.pro`。後者はassertを使うのでdebugでビルドする。具体的なMacコマンドはSunshine側の引き継ぎにある。
- 最初はウィンドウモード・1280×720・30fps要求で、映像、入力、音声、切断再接続を確認する。新しいMacクライアントには初回PIN登録が必要。
- テキスト同期は **TANのSunshine Web UI → Configuration → Inputタブ → Text Clipboard Sharing → Both directions** と **Moonlight → Settings → Sync text clipboard** を有効にする。日本語・改行・絵文字を両方向で確認し、OFF・片方向・切断後の停止も確認する。
- TANの低帯域・背景粗粒化はホスト側で加工する。クライアントにWindowsのD3Dシェーダーを移植する必要はない。Macホストへ同じ機能を追加する場合は別の移植作業となる。
- 移動回線の第2修正、低帯域時のキー・クリックの欠落／遅延、Macの実クリップボードは未確認事項として記録する。

## 共有手順

`custom/main` から `codex/macos-client` などの作業ブランチを作る。変更をcommit・pushし、検証後にcustom/mainへ統合する。取り込み後は `git submodule sync --recursive` と `git submodule update --init --recursive` を実行する。共通ライブラリを変更するときは先に共通リポジトリへpushし、取得可能なコミットを両側へ固定する。

ビルド成果物、libs、ログ、設定・ペアリングをGitで共有しない。TANの証明書や秘密鍵をMacクライアントへコピーしない。
