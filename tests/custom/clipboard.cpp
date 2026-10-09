#undef NDEBUG
#include "../../app/streaming/clipboardstate.h"
#include <cassert>
#include <iostream>

int main() {
    ClipboardState state;
    int sends = 0, applies = 0, wakes = 0;
    auto send = [&](const QByteArray&) { ++sends; return true; };
    auto apply = [&](const QByteArray&) { ++applies; return true; };
    auto wake = [&] { ++wakes; return true; };
    state.configure(false, 1, 3);
    assert(!state.send("private", send));
    state.configure(true, 0, 3);
    assert(state.directions() == 0);
    state.configure(true, 1, 3);
    state.seedLocal("initial local text");
    assert(!state.send("initial local text", send));
    assert(!state.send(QByteArray(SS_CLIPBOARD_TEXT_MAX + 1, 'a'), send));
    assert(!state.send(QByteArray("a\0b",3), send));
    assert(!state.send("", send));
    assert(state.send(QByteArray(SS_CLIPBOARD_TEXT_MAX, 'a'), send));
    assert(state.send("日本語 😀\r\n", send));
    assert(!state.apply("日本語 😀\r\n", apply));
    assert(!state.apply("host", [](const QByteArray&) { return false; }));
    assert(state.apply("host", apply));
    assert(!state.send("host", send));
    assert(!state.send("retry", [](const QByteArray&) { return false; }));
    assert(state.send("retry", send));
    state.receive("first", 5, wake);
    state.receive("latest", 6, wake);
    assert(wakes == 1);
    assert(state.take() == "latest");
    state.receive("failed wake", 11, [] { return false; });
    assert(state.take().isEmpty());
    state.receive("late", 4, wake);
    state.stop();
    assert(state.take().isEmpty());
    state.receive("late", 4, wake);
    assert(state.take().isEmpty());
    state.configure(true, 1, SS_CLIPBOARD_HOST_TO_CLIENT);
    assert(!state.send("private", send));
    assert(state.apply("host", apply));
    state.configure(true, 1, SS_CLIPBOARD_CLIENT_TO_HOST);
    assert(!state.apply("host", apply));
    assert(state.send("client", send));
    state.receive("host", 4, wake);
    assert(state.take().isEmpty());
    assert(sends == 4 && applies == 2);
    state.configure(true, 1, 3);
    const auto now = ClipboardState::Clock::now();
    assert(!state.apply("busy", [](const QByteArray&) { return false; }, now));
    assert(state.hasRetry());
    assert(!state.retryApply(apply, now + std::chrono::milliseconds(100)));
    assert(state.retryApply(apply, now + std::chrono::milliseconds(300)));
    assert(!state.hasRetry());
    assert(!state.send("busy", send));
    assert(!state.apply("expired", [](const QByteArray&) { return false; }, now));
    assert(!state.retryApply(apply, now + std::chrono::seconds(3)));
    assert(!state.hasRetry());
    assert(!state.apply("pending", [](const QByteArray&) { return false; }, now));
    state.stop();
    assert(!state.hasRetry());
    std::cout << "Clipboard client lifecycle tests passed\n";
}
