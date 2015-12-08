package com.baidu.music.classlib.manager;

import java.util.concurrent.ConcurrentHashMap;

/**
 * 基础管理类
 * Created by Jarlene on 2015/12/7.
 */
public abstract class BaseManager<K, V> {
    protected final ConcurrentHashMap<K, V> patchMap = new ConcurrentHashMap<K, V>();
    protected final Object lockObj = new Object();

    /**
     * 添加Item
     * @param key
     * @param value
     */
    public void addItem(K key, V value){
        synchronized (lockObj) {
            patchMap.put(key, value);
        }
    }

    /**
     * 移除Item
     * @param key
     */
    public void removeItem(K key){
        synchronized (lockObj) {
            if (patchMap.containsKey(key)) {
                patchMap.remove(key);
            }
        }
    }

    /**
     * 获取Map
     * @return
     */
    public ConcurrentHashMap<K, V> getHashmap(){
        return patchMap;
    }

    /**
     * 得到Item
     */
    public V getItem(K key){
        synchronized (lockObj) {
            return patchMap.get(key);
        }
    }
}
