// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
template<typename KeyType, typename ValueType>
class TSortedPairArray
{
public:
    // 기본 생성자
    TSortedPairArray() {}

    // 복사 생성자
    TSortedPairArray(const TSortedPairArray<KeyType, ValueType>& Other)
    {
        Pairs = Other.Pairs;
    }

    // 이동 생성자
    TSortedPairArray(TSortedPairArray<KeyType, ValueType>&& Other) noexcept
    {
        Pairs = MoveTemp(Other.Pairs);
    }

    // 초기화 리스트를 받는 생성자
    TSortedPairArray(std::initializer_list<TPair<KeyType, ValueType>> InitList)
    {
        Pairs.Append(InitList);
    }

    // 배열을 받아 초기화하는 생성자
    TSortedPairArray(const TArray<TPair<KeyType, ValueType>>& InPairs)
    {
        Pairs = InPairs;
    }

    int32 Num() const
    {
        return Pairs.Num();
    }

    bool ContainsKey(const KeyType& Key) const
    {
        for (const TPair<KeyType, ValueType>& Pair : Pairs)
        {
            if (Pair.Key == Key)
            {
                return true;
            }
        }
        return false;
    }

    bool ContainsValue(const ValueType& Value) const
    {
        for (const TPair<KeyType, ValueType>& Pair : Pairs)
        {
            if (Pair.Value == Value)
            {
                return true;
            }
        }
        return false;
    }

    // [] 연산자 오버로딩
    ValueType& operator[](const KeyType& Key)
    {
        for (TPair<KeyType, ValueType>& Pair : Pairs)
        {
            if (Pair.Key == Key)
            {
                return Pair.Value;
            }
        }

        // 키가 없으면 새로 추가
        Pairs.Add(TPair<KeyType, ValueType>(Key, ValueType()));
        return Pairs.Last().Value;
    }

    const ValueType& operator[](const KeyType& Key) const
    {
        for (const TPair<KeyType, ValueType>& Pair : Pairs)
        {
            if (Pair.Key == Key)
            {
                return Pair.Value;
            }
        }

        // 키가 없으면 에러 발생
        checkf(false, TEXT("Key not found: %s"), *Key.ToString());
        return *reinterpret_cast<const ValueType*>(nullptr); // 비정상적인 종료를 막기 위한 더미 반환
    }

    auto begin() { return Pairs.GetData(); }
    auto end() { return Pairs.GetData() + Pairs.Num(); }

    auto begin() const { return Pairs.GetData(); }
    auto end() const { return Pairs.GetData() + Pairs.Num(); }

    // 키와 값을 추가
    void Add(const KeyType& Key, const ValueType& Value)
    {
        for (TPair<KeyType, ValueType>& Pair : Pairs)
        {
            if (Pair.Key == Key)
            {
                checkf(false, TEXT("Duplicate key"));
                return;
            }
        }
        Pairs.Add(TPair<KeyType, ValueType>(Key, Value));
    }

    // 키로 값을 제거
    bool Remove(const KeyType& Key)
    {
        for (int32 i = 0; i < Pairs.Num(); ++i)
        {
            if (Pairs[i].Key == Key)
            {
                Pairs.RemoveAt(i);
                return true;
            }
        }
        return false;
    }

    // 키로 값을 찾음
    ValueType* Find(const KeyType& Key)
    {
        for (TPair<KeyType, ValueType>& Pair : Pairs)
        {
            if (Pair.Key == Key)
            {
                return &Pair.Value;
            }
        }
        return nullptr;
    }

    bool TryFind(const KeyType& Key, ValueType*& OutValue) const
    {
        for (const TPair<KeyType, ValueType>& Pair : Pairs)
        {
            if (Pair.Key == Key)
            {
                OutValue = &Pair.Value;
                return true;
            }
        }
        OutValue = nullptr;
        return false;
    }

    void Empty()
    {
        Pairs.Empty();
    }

    // 키 목록을 반환
    TArray<KeyType> GetKeys() const
    {
        TArray<KeyType> Keys;
        for (const TPair<KeyType, ValueType>& Pair : Pairs)
        {
            Keys.Add(Pair.Key);
        }
        return Keys;
    }

    // 값 목록을 반환
    TArray<ValueType> GetValues() const
    {
        TArray<ValueType> Values;
        for (const TPair<KeyType, ValueType>& Pair : Pairs)
        {
            Values.Add(Pair.Value);
        }
        return Values;
    }

    // 모든 키-값 쌍을 반환
    const TArray<TPair<KeyType, ValueType>>& GetAllPairs() const
    {
        return Pairs;
    }

    // 오름차순/내림차순 정렬하는 SortKey 메서드
    void SortKey(bool bAscending = true)
    {
        Pairs.Sort([bAscending](const TPair<KeyType, ValueType>& A, const TPair<KeyType, ValueType>& B)
            {
                return bAscending ? A.Key < B.Key : A.Key > B.Key;
            });
    }

    // 사용자 정의 정렬 로직
    void Sort(TFunctionRef<bool(const TPair<KeyType, ValueType>&, const TPair<KeyType, ValueType>&)> SortPredicate)
    {
        Pairs.Sort(SortPredicate);
    }

    // 첫 번째/마지막 키-값 쌍 반환
    const TPair<KeyType, ValueType>* FirstPair() const
    {
        return Pairs.Num() > 0 ? &Pairs[0] : nullptr;
    }

    const TPair<KeyType, ValueType>* LastPair() const
    {
        return Pairs.Num() > 0 ? &Pairs.Last() : nullptr;
    }

    // 첫 번째/마지막 키 반환
    const KeyType* FirstKey() const
    {
        return Pairs.Num() > 0 ? &Pairs[0].Key : nullptr;
    }

    const KeyType* LastKey() const
    {
        return Pairs.Num() > 0 ? &Pairs.Last().Key : nullptr;
    }

    // 첫 번째/마지막 값 반환
    const ValueType* FirstValue() const
    {
        return Pairs.Num() > 0 ? &Pairs[0].Value : nullptr;
    }

    const ValueType* LastValue() const
    {
        return Pairs.Num() > 0 ? &Pairs.Last().Value : nullptr;
    }

private:
    TArray<TPair<KeyType, ValueType>> Pairs;
};
