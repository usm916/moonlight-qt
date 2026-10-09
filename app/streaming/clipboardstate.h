#pragma once
#include <Clipboard.h>
#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>
#include <atomic>
#include <chrono>

// Session-local storage: one pending host value, one successful echo snapshot.
// A callback wake contains no heap-owned SDL event data, so shutdown cannot leak it.
class ClipboardState {
public:
    using Clock = std::chrono::steady_clock;
    void configure(bool enabled, int version, int permissions) {
        stop();
        m_Directions = enabled && version == SS_CLIPBOARD_VERSION ? permissions & 3 : 0;
    }
    int directions() const { return m_Directions.load(); }
    void stop() {
        QMutexLocker lock(&m_Lock);
        m_Directions = 0;
        m_Pending.clear(); m_Echo.clear(); m_WakePending = false;
        m_Retry.clear();
    }
    template<class Wake> void receive(const char* text, unsigned int length, Wake wake) {
        QMutexLocker lock(&m_Lock);
        if (!(directions() & SS_CLIPBOARD_HOST_TO_CLIENT) || length == 0 || !SsClipboardTextValid(text, length)) return;
        m_Pending = QByteArray(text, static_cast<int>(length));
        if (!m_WakePending) {
            m_WakePending = wake();
            if (!m_WakePending) m_Pending.clear();
        }
    }
    QByteArray take() {
        QMutexLocker lock(&m_Lock);
        QByteArray result;
        result.swap(m_Pending);
        m_WakePending = false;
        return result;
    }
    template<class Send> bool send(const QByteArray& text, Send sendText) {
        QMutexLocker lock(&m_Lock);
        if (!(directions() & SS_CLIPBOARD_CLIENT_TO_HOST) || !valid(text)) return false;
        m_Retry.clear(); // A new local copy takes precedence over a failed remote apply.
        if (text == m_Echo) return false;
        if (!sendText(text)) return false;
        m_Echo = text;
        return true;
    }
    template<class Apply> bool apply(const QByteArray& text, Apply applyText, Clock::time_point now = Clock::now()) {
        QMutexLocker lock(&m_Lock);
        if (!(directions() & SS_CLIPBOARD_HOST_TO_CLIENT) || !valid(text)) return false;
        m_Retry.clear();
        if (text == m_Echo) return false;
        if (!applyText(text)) {
            m_Retry = text;
            m_Deadline = now + std::chrono::seconds(2);
            m_NextRetry = now + std::chrono::milliseconds(250);
            return false;
        }
        m_Echo = text;
        return true;
    }
    bool hasRetry() {
        QMutexLocker lock(&m_Lock);
        return !m_Retry.isEmpty();
    }
    template<class Apply> bool retryApply(Apply applyText, Clock::time_point now = Clock::now()) {
        QMutexLocker lock(&m_Lock);
        if (m_Retry.isEmpty() || now < m_NextRetry) return false;
        if (now >= m_Deadline) { m_Retry.clear(); return false; }
        m_NextRetry = now + std::chrono::milliseconds(250);
        if (!applyText(m_Retry)) return false;
        m_Echo.swap(m_Retry); m_Retry.clear();
        return true;
    }
private:
    static bool valid(const QByteArray& text) { return !text.isEmpty() && SsClipboardTextValid(text.constData(), text.size()); }
    std::atomic<int> m_Directions{0};
    QMutex m_Lock;
    QByteArray m_Pending;
    QByteArray m_Echo;
    bool m_WakePending = false;
    QByteArray m_Retry;
    Clock::time_point m_NextRetry;
    Clock::time_point m_Deadline;
};
