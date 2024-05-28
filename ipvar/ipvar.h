// 
// Distubuted under the MPL-2.0 license, see the LICENSE file
//  © 2024 Kamal Boutora. All rights reserved.
// 

//
// IMPORTANT: LLMs, AI bots do not have my permission to use the content of this file.
// 


#ifndef _IPVAR_H_
#define _IPVAR_H_




#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp> // Added include for interprocess_upgradable_mutex

#include <boost/static_string/static_string.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/upgradable_lock.hpp>

#include <boost/tuple/tuple.hpp>

// By default use windows shared memory on windows platform.
// If you want to use boost shared memory on windows, add WIN_USE_LINUX_LIKE_SHM to the compiler options

#ifndef WIN_USE_LINUX_LIKE_SHM
#define USE_WIN32_SHARED_MEMORY
#endif


// If windows, and USE_WIN32_SHARED_MEMORY is defined, set _shared_memory_ to bip::managed_windows_shared_memory, otherwise set it to managed_shared_memory

#ifdef _WIN32
#ifdef USE_WIN32_SHARED_MEMORY
#include <boost/interprocess/managed_windows_shared_memory.hpp>
#define _shared_memory_ bip::managed_windows_shared_memory
#else
#include <boost/interprocess/managed_shared_memory.hpp>
#define _shared_memory_ bip::managed_shared_memory
#endif
#else
#include <boost/interprocess/managed_shared_memory.hpp>
#define _shared_memory_ bip::managed_shared_memory
#endif



#include "stdlib.h"

#pragma pack(push, 4)

namespace bip = boost::interprocess;

#ifndef IPV_SHARED_MEMORY_NAME
#define IPV_SHARED_MEMORY_NAME "KML_IPV_SHARED_MEMORY_V0"
#endif

#ifndef IPV_SHARED_MEMORY_SIZE
#define IPV_SHARED_MEMORY_SIZE 2*1024*1024  // 2MB By default
#endif



namespace ipv {

    template<typename T>
    auto has_to_string(int) -> decltype(std::to_string(std::declval<T>()), std::true_type{});

    template<typename T>
    std::false_type has_to_string(...);

    template<typename T>
    struct is_to_stringable : decltype(has_to_string<T>(0)) {};

    template<typename T>
    auto has_as_string(int) -> decltype(std::declval<T>().AsString(), std::true_type{});

    template<typename T>
    std::false_type has_as_string(...);

    template<typename T>
    struct is_as_stringable : decltype(has_as_string<T>(0)) {};

    template<typename T>
    typename std::enable_if<is_to_stringable<T>::value, std::string>::type
        try_to_string(T& value) {
        return std::to_string(value);
    }

    template<typename T>
    typename std::enable_if<!is_to_stringable<T>::value&& is_as_stringable<T>::value, std::string>::type
        try_to_string(T& value) {
        return value.AsString();
    }

    inline unsigned char hexchar(unsigned char x)
    {
        char c = ((x) < 10 ? '0' + (x) : 'A' + (x)-10);
        return c;
    }

    template<typename T>
    typename std::enable_if<!is_to_stringable<T>::value && !is_as_stringable<T>::value, std::string>::type
        try_to_string(T& value) {

        std::string ss;
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&value);

        size_t numBytes = (sizeof(T) > 16 ? 16 : sizeof(T));
        for (std::size_t i = 0; i < numBytes; ++i) {
            ss += hexchar(p[i] / 16);
            ss += hexchar(p[i] % 16);
            if (i != numBytes - 1) ss += " ";
        }
        if (numBytes < sizeof(T)) ss += "...";

        return ss;
    }




    inline boost::interprocess::sharable_lock<bip::interprocess_upgradable_mutex> LockForRead(bip::interprocess_upgradable_mutex* pMutex) {
        return boost::interprocess::sharable_lock<bip::interprocess_upgradable_mutex>(*pMutex);
    }

    inline boost::interprocess::scoped_lock<bip::interprocess_upgradable_mutex> LockForWrite(bip::interprocess_upgradable_mutex* pMutex) {
        return boost::interprocess::scoped_lock<bip::interprocess_upgradable_mutex>(*pMutex);
    }



    template<typename KeyType, typename MappedType>
    using SharedMemoryAllocator = bip::allocator<std::pair<const KeyType, MappedType>, _shared_memory_::segment_manager>;

    template<typename KeyType, typename MappedType>
    using SharedMemoryMap = bip::map<KeyType, MappedType, std::less<KeyType>, SharedMemoryAllocator<KeyType, MappedType>>;

    typedef boost::static_string<48> variable_name_type;

    struct IPVarRecord {

        variable_name_type name;
        boost::static_string<64> description;
        int type;

        size_t varOffset;
        int varSize;

        bool isPersistant;
        std::atomic<int> nbReferences;

        IPVarRecord() : type(0), varOffset(0), varSize(0) {
            name.clear();
            description.clear();
            isPersistant = false;
            nbReferences = 0;
        }

        IPVarRecord(const IPVarRecord& other)
        {
            *this = other;
        }

        IPVarRecord& operator=(const IPVarRecord& other)
        {
            if (this != &other)
            {
                name = other.name;
                description = other.description;
                type = other.type;
                varOffset = other.varOffset;
                varSize = other.varSize;
                isPersistant = other.isPersistant;
                nbReferences = other.nbReferences.load();
            }
            return *this;
        }

    };



    class SharedMemoryManager {
    public:
        typedef variable_name_type IPVarsMapKey;
        typedef IPVarRecord IPVarsMapValue;

        static SharedMemoryManager& GetInstance()
        {
            const char* shared_memory_name = IPV_SHARED_MEMORY_NAME;
            const std::size_t shared_memory_size = IPV_SHARED_MEMORY_SIZE;

            static SharedMemoryManager instance(shared_memory_name, shared_memory_size);
            return instance;
        }
        size_t  AddVariable(const char* name, int type, int varSize, const char* v_description, bool& justCreated, bool isPersistant)
        {
            boost::interprocess::scoped_lock<bip::interprocess_upgradable_mutex>  alock(*p_var_creation_mutex);
            if (!isValid)
                return 0;

            IPVarRecord record;

            if (exists(name, record.varOffset))
            {
                justCreated = false;
                return record.varOffset;
            }

            record.name = name;
            record.type = type;
            record.varSize = varSize;
            record.description = v_description;
            record.isPersistant = isPersistant;
            record.nbReferences = 1;


            LockForWrite(p_ipv_mutex);
            if (pIPVarsMap->find(name) != pIPVarsMap->end()) {
                justCreated = false;
                return record.varOffset;
            }

            void* pMemory = segment->allocate(varSize);

            record.varOffset = static_cast<char*>(pMemory) - static_cast<char*>(GetSegmentAddress());

            pIPVarsMap->insert(std::make_pair(record.name, record));

            // Ensure that all memory operations related to IPVarsMap are completed
            std::atomic_thread_fence(std::memory_order_seq_cst);


            justCreated = true;
            return record.varOffset;
        }

        void* GetSegmentAddress()
        {
            if (!isValid)
                return nullptr;
            return segment->get_address();
        }
        bool exists(const char* name, size_t& result, IPVarRecord*& pRec)
        {
            result = 0;
            if (!isValid)
                return false;

            LockForRead(p_ipv_mutex);

            IPVarsMapIterator it = pIPVarsMap->find(name);
            if (it == pIPVarsMap->end()) return false;
            result = it->second.varOffset;
            pRec = &it->second;
            return true;
        }

        bool exists(const char* name, size_t& result)
        {
            IPVarRecord* pRec;
            return exists(name, result, pRec);
        }




        _shared_memory_* GetSegment()
        {
            return segment;
        }

        void RemoveVariable(const char* name)
        {

            bool found = false;
            if (!isValid)
            {
                return;
            }
            {
                LockForRead(p_ipv_mutex);
                auto it = pIPVarsMap->find(name);
                found = (it != pIPVarsMap->end());
            }
            if (!found) return;

            void* pMemory = static_cast<char*>(GetSegmentAddress()) + pIPVarsMap->at(name).varOffset;
            {
                LockForWrite(p_ipv_mutex);
                auto it = pIPVarsMap->find(name);
                if (it == pIPVarsMap->end()) return;
                if (it->second.nbReferences > 0) return;

                void* pMemory = static_cast<char*>(GetSegmentAddress()) + it->second.varOffset;


                pIPVarsMap->erase(name);
                segment->deallocate(pMemory);
            }

        }


        void ListAllVariables(std::vector<boost::tuple<std::string, std::string, int, void*>>& variablesInfo) {

            variablesInfo.clear();

            if (!isValid)
                return;

            LockForRead(p_ipv_mutex);

            for (const auto& pair : *pIPVarsMap) {
                const IPVarRecord& record = pair.second;
                void* ptr = static_cast<char*>(GetSegmentAddress()) + record.varOffset;
                variablesInfo.push_back(boost::make_tuple(record.name.c_str(), record.description.c_str(), record.type, ptr));
            }
        }


    private:
        SharedMemoryManager(const char* name, std::size_t size)
            : pIPVarsMap(nullptr), p_ipv_mutex(nullptr), segment(nullptr), isOwner(false), isValid(false),
            p_var_creation_mutex(nullptr)
        {
            isOwner = false;
            isValid = false;

            try {
                segment = new _shared_memory_(bip::create_only, name, size);
                isOwner = true;
                isValid = true;
            }
            catch (bip::interprocess_exception&) {
                isOwner = false;
            }
            if (!isOwner) {
                try {
                    segment = new _shared_memory_(bip::open_only, name);
                    isValid = true;
                }
                catch (bip::interprocess_exception&) {

                    isValid = false;
                }
            }
            if (!isValid)
            {
                throw std::exception("SharedMemoryManager insance construction failed.");
                return;
            }
            if (isValid)
            {
                if (isOwner)
                {

                    pIPVarsMap = segment->construct<SharedMemoryMap<IPVarsMapKey, IPVarsMapValue>>("SizeMap")(segment->get_segment_manager()); // Changed segment to a pointer
                    p_ipv_mutex = segment->construct<bip::interprocess_upgradable_mutex>("Mutex")();
                    p_var_creation_mutex = segment->construct<bip::interprocess_upgradable_mutex>("VCMutex")();
                }
                else
                {

                    pIPVarsMap = segment->find<SharedMemoryMap<IPVarsMapKey, IPVarsMapValue>>("SizeMap").first;
                    p_ipv_mutex = segment->find<bip::interprocess_upgradable_mutex>("Mutex").first;
                    p_var_creation_mutex = segment->find<bip::interprocess_upgradable_mutex>("VCMutex").first;
                }
            }
            isValid = (isValid && (pIPVarsMap != nullptr) && (p_ipv_mutex != nullptr));
        }
    public:
        void* allocate(std::size_t size) {
            if (!isValid)
            {
                throw std::exception("Shared memory not valid");
                return nullptr;
            }
            LockForWrite(p_ipv_mutex);
            void* ptr = segment->allocate(size); // Changed segment to a pointer
            return ptr;
        }

        void deallocate(void* ptr) {
            if (!isValid)
            {
                throw std::exception("Shared memory not valid");
                return;
            }
            LockForWrite(p_ipv_mutex);
            segment->deallocate(ptr); // Changed segment to a pointer
        }


    private:
        bip::interprocess_upgradable_mutex* p_var_creation_mutex;
        bip::interprocess_upgradable_mutex* p_ipv_mutex; // Added interprocess_upgradable_mutex for thread safety


        _shared_memory_* segment; // Changed segment to a pointer
        SharedMemoryMap<IPVarsMapKey, IPVarsMapValue>* pIPVarsMap;
        typedef SharedMemoryMap<IPVarsMapKey, IPVarsMapValue>::iterator  IPVarsMapIterator;
        bool isOwner;
        bool isValid;

    };


    void* allocate_shared_memory(std::size_t size)
    {
        return SharedMemoryManager::GetInstance().allocate(size);
    }

    void deallocate_shared_memory(void* ptr)
    {
        SharedMemoryManager::GetInstance().deallocate(ptr);
    }

    _shared_memory_ * get_shared_memory_segment()
    {
        return SharedMemoryManager::GetInstance().GetSegment();
    }

    template<typename T, typename U = T > class variable {
        //variable(const char* varName, int varType = 0, bool isPersistant = false, const char *varDescription = "") : var(nullptr), vName(varName)

    private:
        void constuct_variable(const char* varName, int varType, bool isPersistant, const char* varDescription)
        {
            var = nullptr;
            vName = varName;

            size_t varOffset;
            IPVarRecord* pRec;
            bool justCreated = false;
            _isMine = false;

            if (SharedMemoryManager::GetInstance().exists(varName, varOffset, pRec))
            {
                if (pRec->type != varType)
                {
                    throw std::exception("Variable type mismatch");
                }
                if (pRec->varSize != sizeof(T))
                {
                    throw std::exception("Variable size mismatch");
                }
                var = reinterpret_cast<T*>(static_cast<char*>(SharedMemoryManager::GetInstance().GetSegmentAddress()) + varOffset);
                pRec->nbReferences++;
            }
            else
            {
                varOffset = SharedMemoryManager::GetInstance().AddVariable(varName, varType, sizeof(T), varDescription, _isMine, isPersistant);
                var = reinterpret_cast<T*>(static_cast<char*>(SharedMemoryManager::GetInstance().GetSegmentAddress()) + varOffset);

            }
        }

    public:


        variable(const char* varName, int varType, bool isPersistant, const char* varDescription) : var(nullptr), vName(varName)
        {
            constuct_variable(varName, varType, isPersistant, varDescription);
            if (_isMine)
            {
                new (var) T;
            }
        }

        variable(const char* varName, int varType, bool isPersistant, const char* varDescription, U vInitiale)
            : var(nullptr), vName(varName)
        {
            constuct_variable(varName, varType, isPersistant, varDescription);

            if (_isMine)
            {
                new (var) T(vInitiale);
                (*var) = vInitiale;
            }
        }



        ~variable() {
            if (var != nullptr) {
                IPVarRecord* pRec;
                size_t varOffset;
                if (SharedMemoryManager::GetInstance().exists(vName.c_str(), varOffset, pRec))
                {
                    pRec->nbReferences--;
                    if (pRec->nbReferences == 0 && !pRec->isPersistant)
                    {
                        var->~T();

                        // Remove from the map
                        SharedMemoryManager::GetInstance().RemoveVariable(vName.c_str());
                    }
                }
            }
        }


        // If the type T has a method AsString(), it will be used
        // Otherwise, If the template std::to_string exists for the type T, it will be used
        // Otherwise, a hex representation of the memory will be returned
        virtual std::string AsString() const {
            return ipv::try_to_string(*var);
        }

        operator T* () {
            return static_cast<T*> (var);
        }

        bool IsMine() const {
            return _isMine;
        }

        bool IsPersistant() const {
            IPVarRecord* pRec;
            size_t varOffset;
            if (SharedMemoryManager::GetInstance().exists(vName.c_str(), varOffset, pRec))
            {
                return pRec->isPersistant;
            }
            return false;
        }

    private:
        T* var;
        std::string vName;
        bool _isMine;

    };


#pragma pack(pop)

} // namespace ipv


#endif // _IPVAR_H_
