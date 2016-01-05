package com.baidu.music.classlib.manager;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * 基础管理类
 * Created by Jarlene on 2015/12/7.
 */
public abstract class BaseManager<K, V> {
    protected final Map<K, V> patchMap = Collections.synchronizedMap(new HashMap<K, V>());

    /**
     * 添加Item
     * @param key
     * @param value
     */
    public void addItem(K key, V value){
        patchMap.put(key, value);
    }

    /**
     * 移除Item
     * @param key
     */
    public void removeItem(K key){
        if (patchMap.containsKey(key)) {
            patchMap.remove(key);
        }
    }

    /**
     * 获取Map
     * @return
     */
    public Map<K, V> getHashmap(){
        return patchMap;
    }

    /**
     * 得到Item
     */
    public V getItem(K key){
        return patchMap.get(key);
    }

    /**
     * 是否包含key
     * @param key
     * @return
     */
    public boolean isContainKey(K key) {
        return patchMap.containsKey(key);
    }

    /**
     * 是否包含Value
     * @param value
     * @return
     */
    public boolean isContainValue(V value) {
        return patchMap.containsValue(value);
    }

}
