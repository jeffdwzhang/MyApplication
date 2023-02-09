//
// Created by 张德文 on 2022/5/21.
//

#ifndef ANDROIDAVLEARN_RUNNABLE_H
#define ANDROIDAVLEARN_RUNNABLE_H

struct Runnable {
    virtual ~Runnable() {}
    virtual void run() = 0;
};

template<class T>
class RunnableFunctor : public Runnable {
public:
    RunnableFunctor(const T& f) : m_func(f) {}
    virtual void run() { m_func(); }

private:
    T m_func;
};

template<class T>
class RunnableFunctor<T*> : public Runnable {
public:
    RunnableFunctor(const T* f) : m_func(f) {}
    virtual void run() { (*m_func)(); }
private:
    RunnableFunctor(const RunnableFunctor&);
    RunnableFunctor& operator=(const RunnableFunctor&);

private:
    T* m_func;
};

template <>
class RunnableFunctor<Runnable> : public Runnable {
    RunnableFunctor();
};

template<>
class RunnableFunctor<Runnable*> : public Runnable {
public:
    RunnableFunctor(Runnable* f) : m_func(f) {}
    virtual void run() { static_cast<Runnable*>(m_func)->run(); }
private:
    RunnableFunctor(const RunnableFunctor&);
    RunnableFunctor& operator=(const RunnableFunctor&);

private:
    Runnable* m_func;
};

template <class T>
struct TransformImplement {
    static Runnable* transform(const T& t) {
        return new RunnableFunctor<T>(t);
    }
};

template <class T>
inline Runnable* transform(const T& t) {
    return TransformImplement<T>::transform(t);
}

#endif //ANDROIDAVLEARN_RUNNABLE_H
