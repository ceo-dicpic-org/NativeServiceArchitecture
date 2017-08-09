#pragma once

#include <mutex>
#include <condition_variable>
#include <limits>
#include <queue>
#include <chrono>

namespace NSA
{

/*!
 * @brief Blocking topped queue implementation.
 * @details This queue has blocking and topped features. Each pop and
 *          push operation is thread safe. The queue has a top, wich
 *          means that there can only be a specific maximum ammount of
 *          elements in the queue. Whenever the top is reached, each
 *          further push operation will block. Whenever the bottom is
 *          reached, each further pop operation will block. Both
 *          operations will unblock as soon as a element is added,
 *          or removed.
 *          The queue itself does not store any elements. It only
 *          takes pointers to existing elements.
 *          
 * @author Gert-Jan Rozing (Gert.Rozing@myestro.de)
 * @date 08.2016
 * @copyright "THE BEER-WARE LICENSE" (Revision 42)"
 */
template <class T>
class BlockingQueue
{
public:
    BlockingQueue(const std::size_t maxItems = 0);

    /**
     * @brief Blocking and waiting push.
     * @details First the function checks, if the maximum of the queue is
     *          reached. If true the function will block until a item has
     *          been poppen out of the queue.
     *          If the timeout is reached, the push will be rejected. 
     *          Afterwards the function will block the queue and add the
     *          src parameter into the queue.
     * 
     * @param src A new item to push into the queue.
     * @param timeOut A duration after which the push will time out.
     * @return True on success. False if a timeout happend.
     */
    bool push(T src, const std::chrono::milliseconds timeOut = std::chrono::milliseconds(30));

    /**
     * @brief Blocking and waiting pop.
     * @details First the function checks if the queue is empty. If true
     *          the function will block until a item is placed into the
     *          queue.
     *          Afterwards the function will block the queue and pops and
     *          stores the first element into the dst parameter.
     * 
     * @param dst A pointer to the storage of the popped element.
     * @return True on success. False if dst is nullptr.
     */
    bool pop(T *dst);

    /**
     * @brief Blocking getter for the current size of the queue.
     * @details Quickly blocks the queue to check the size.
     * @return The current size of the queue.
     */
    const size_t size() const;

    /**
     * @brief Blocking check to see if the queue is empty.
     * @details Quickly blocks the queue to check if it is empty.
     * @return True if empty, else false.
     */
    const bool empty() const;

    /**
     * @brief Getter for the maximum size of the queue.
     * @return The maximum size of the queue.
     */
    const std::size_t max() const;

private:
    std::queue<T> queue; 
    const std::size_t maxItems;
    mutable std::mutex queueMutex;
    mutable std::mutex waitMutex;
    mutable std::condition_variable waitCondition;
};

/// Implementation.

template <class T>
BlockingQueue<T>::BlockingQueue(const std::size_t maxItems) :
    maxItems(maxItems <= 0 ? std::numeric_limits<std::size_t>::max() : maxItems)
{}

template <class T>
bool BlockingQueue<T>::push(T src, const std::chrono::milliseconds timeOut)
{
    std::unique_lock<std::mutex> waitLock(waitMutex);

    if (waitCondition.wait_for(waitLock, timeOut, [this]{return queue.size() < maxItems;}))
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        queue.push(src);
        waitCondition.notify_all();

        return true;
    }   

    return false;     
}

template <class T>
bool BlockingQueue<T>::pop(T *dst)
{
    if (dst == nullptr)
        return false;

    std::unique_lock<std::mutex> waitLock(waitMutex);

    waitCondition.wait(waitLock, [this]{return !queue.empty();});

    std::lock_guard<std::mutex> lock(queueMutex);
    *dst = queue.front();
    queue.pop();
    waitCondition.notify_all();        

    return true;
}

template <class T>
const size_t BlockingQueue<T>::size() const
{
    std::lock_guard<std::mutex> lock(queueMutex);
    return queue.size();
}

template <class T>
inline const std::size_t BlockingQueue<T>::max() const
{
    return maxItems;
}

template <class T>
const bool BlockingQueue<T>::empty() const
{
    std::lock_guard<std::mutex> lock(queueMutex);
    return queue.empty();
}

} // namespace NSA