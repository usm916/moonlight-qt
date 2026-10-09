# Moonlight Qt Custom

Sunshine Customと連携するWindows向けクライアントです。Windows/Linux/macOS対応のMoonlight Qtをベースにし、最初はWindows同士のテキストクリップボード同期を目標とします。

| 項目 | 設定 |
| --- | --- |
| 本流 | [moonlight-stream/moonlight-qt](https://github.com/moonlight-stream/moonlight-qt)（`upstream`） |
| 独自フォーク | [usm916/moonlight-qt](https://github.com/usm916/moonlight-qt)（`origin`） |
| 統合ブランチ | `custom/main` |
| 本流追跡ブランチ | ローカル`master` → `upstream/master` |
| 初期ベース | `a57e947d7185fd01384f6973df4df59ae2968d01` |
| ホスト | [usm916/Sunshine](https://github.com/usm916/Sunshine/tree/custom/main) |
| 共通ライブラリ | [usm916/moonlight-common-c](https://github.com/usm916/moonlight-common-c/tree/custom/main) |

既存フォークの`master`は`9cb4105aecbd4df7be09ec03da531720d8fd1f86`で、本流から509コミット遅れ、独自コミットはありませんでした。古い`master`は保持し、新しい開発は`custom/main`で行います。GitHubの既定ブランチは古い`master`のままです。

## 現在の状態

`feature/clipboard-text`にWindows向けテキスト同期を実装しました。既定OFFで、対応ホストの許可方向に従います。画像・ファイル転送は対象外です。Windows Releaseビルドと通信・状態管理テストは成功しました。別端末間の配信を通した往復と他OSは未検証です。

共通ライブラリはホストと同じ固定コミットを使います。そのコミットには[通信仕様](moonlight-common-c/moonlight-common-c/docs/custom/clipboard-v1.md)があり、通信に関する決定はこの文書へ集約します。依存バージョンは[compatibility.json](docs/custom/compatibility.json)に記録します。

Sunshine側の`scripts/check-custom-pair.ps1`でホスト・クライアントの固定コミット、submoduleの取得元とチェックアウト、通常配信のソース基準を検査できます。この検査はネイティブビルドやコピー動作の成功を保証しません。

## 作業フォルダ

```text
D:\data\code\sunshineCustom       # Windowsホスト
D:\data\code\moonlightCustom      # Qtクライアント
D:\data\code\moonlightCommonCustom # 共通通信ライブラリ
```

各フォルダは独立したGitリポジトリです。共通ライブラリの開発用フォルダを直接参照するのではなく、ホスト・クライアントのsubmoduleに取得可能なコミットを固定します。

## 独自変更と本流更新

機能は`custom/main`から`feature/clipboard-text`などのブランチを作って実装し、検証後にmergeします。

本流更新は作業ツリーを空にしてから、各コマンドの成功を確認しながら実行します。

```powershell
git fetch upstream --tags
git switch master
git merge --ff-only upstream/master
git switch custom/main
git switch -c integration/upstream-YYYYMMDD
git merge --no-ff upstream/master
git submodule sync --recursive
git submodule update --init --recursive
# 共有ライブラリの固定値をホスト側と揃え、ビルド・接続・互換性を確認
git switch custom/main
git merge --ff-only integration/upstream-YYYYMMDD
git push origin custom/main
```

`YYYYMMDD`は実施日へ置き換えます。共通ライブラリを本流の参照値に戻す変更が入った場合は、採用する固定値を両側で評価して更新します。公開済みの統合ブランチをrebaseしません。

## Windowsビルドと検証

本流の[README](README.md#building)に従ってQt MSVC版、Visual Studio、依存ライブラリを準備します。SunshineのMSYS2ビルド環境とは別です。

Windows x64でVisual Studio 2022 Build Tools 17.14.13、Qt 6.11.1 MSVC2022、jom 1.1.4、上流のWindows依存パッケージv19を使用してReleaseビルドを確認しました。Qt SDKは`D:\data\tools\Qt`です。

初回は`git submodule update --init --recursive`と本流の`setup-deps.ps1`を使って依存関係を準備します。ビルド手順やバージョン要件は本流の更新に合わせて確認します。

通常配信・切断再接続を基準にし、同期実装後はOFF/ON、未対応ホスト、Unicode、上限超過、反射ループ、クリップボード占有を検証します。他OSは実機確認後に対応状況を更新します。

## カスタム版のビルドと利用

Sunshine側の`scripts/custom/install-qt.py`で公式のQt SDKを取得できます（Pythonの`py7zr`が必要）。上流の`setup-deps.ps1`でWindows依存パッケージを準備してから実行します。

```powershell
& .\scripts\build-custom.ps1 -QtRoot 'D:\data\tools\Qt' -Jobs 4
```

共通プロトコルとクライアントのライフサイクルテストもこのコマンドで実行します。成功時は`build/deploy-custom-x64-release/Moonlight.exe`と必要なDLLを配置します。フォルダ全体がポータブル版です。

設定画面の`Sync text clipboard`を有効にし、カスタムSunshineへ接続します。ホスト→クライアントを許可すると接続時のホストのテキストも同期されます。UTF-8で32,755バイトまで、空の内容は同期しません。詳細は共通仕様を参照してください。
