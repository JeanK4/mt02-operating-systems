#pragma once
#include <string>
#include <list>
#include <iostream>
#include <iomanip>

using namespace std;

// Enum para los algoritmos de asignación
enum AllocationAlgorithm {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
};

// Clase para representar un bloque de memoria
class MemoryBlock {
private:
    int start;        // Posición de inicio
    int size;         // Tamaño del bloque
    int used;         // Cantidad realmente usada (para FI)
    string process;   // Nombre del proceso (vacío si está libre)
    bool isFree;      // true si el bloque está libre
    
public:
    // Constructor
    MemoryBlock(int s, int sz, string p = "", bool free = true, int u = -1) 
        : start(s), size(sz), used(u == -1 ? sz : u), process(p), isFree(free) {}
    
    // Getters
    int getStart() const { return start; }
    int getSize() const { return size; }
    int getUsed() const { return used; }
    string getProcess() const { return process; }
    bool getIsFree() const { return isFree; }
    
    // Setters
    void setStart(int s) { start = s; }
    void setSize(int sz) { size = sz; }
    void setUsed(int u) { used = u; }
    void setProcess(const string& p) { process = p; }
    void setIsFree(bool free) { isFree = free; }
    
    // Método para liberar el bloque
    void free() {
        isFree = true;
        process = "";
    }
    
    // Método para ocupar el bloque
    void allocate(const string& p) {
        isFree = false;
        process = p;
    }

    // Allocate with used value
    void allocate(const string& p, int u) {
        isFree = false;
        process = p;
        used = u;
    }
    
    // Método para incrementar el tamaño (útil para coalescencia)
    void increaseSize(int additionalSize) {
        size += additionalSize;
    }
};

class MemoryManager {
private:
    int totalMemory;
    list<MemoryBlock> memory;
    AllocationAlgorithm algorithm;
    
public:
    MemoryManager(int size, AllocationAlgorithm algo = FIRST_FIT) 
        : totalMemory(size), algorithm(algo) {
        // Inicialmente toda la memoria está libre
        memory.push_back(MemoryBlock(0, size, "", true));
    }
    
    // Asignar memoria a un proceso según el algoritmo seleccionado
    bool allocate(string process, int size) {
        return allocate(process, size, size);
    }

    // Asignar con "used" opcional
    bool allocate(string process, int size, int used) {
        switch (algorithm) {
            case FIRST_FIT:
                return allocateFirstFit(process, size, used);
            case BEST_FIT:
                return allocateBestFit(process, size, used);
            case WORST_FIT:
                return allocateWorstFit(process, size, used);
            default:
                return false;
        }
    }
    
private:
    // First Fit: primer bloque libre que quepa
    bool allocateFirstFit(string process, int size, int used) {
        for (auto it = memory.begin(); it != memory.end(); ++it) {
            if (it->getIsFree() && it->getSize() >= size) {
                allocateBlock(it, process, size, used);
                return true;
            }
        }
        return false;
    }
    
    // Best Fit: bloque libre más pequeño que pueda contener el proceso
    bool allocateBestFit(string process, int size, int used) {
        auto bestFit = memory.end();
        int minSize = totalMemory + 1;
        
        for (auto it = memory.begin(); it != memory.end(); ++it) {
            if (it->getIsFree() && it->getSize() >= size) {
                if (it->getSize() < minSize) {
                    minSize = it->getSize();
                    bestFit = it;
                }
            }
        }
        
        if (bestFit != memory.end()) {
            allocateBlock(bestFit, process, size, used);
            return true;
        }
        return false;
    }
    
    // Worst Fit: bloque libre más grande disponible
    bool allocateWorstFit(string process, int size, int used) {
        auto worstFit = memory.end();
        int maxSize = -1;
        
        for (auto it = memory.begin(); it != memory.end(); ++it) {
            if (it->getIsFree() && it->getSize() >= size) {
                if (it->getSize() > maxSize) {
                    maxSize = it->getSize();
                    worstFit = it;
                }
            }
        }
        
        if (worstFit != memory.end()) {
            allocateBlock(worstFit, process, size, used);
            return true;
        }
        return false;
    }
    
    // Método auxiliar para asignar un bloque
    void allocateBlock(list<MemoryBlock>::iterator it, string process, int size, int used) {
        if (it->getSize() == size) {
            // El hueco es exactamente del tamaño necesario
            it->allocate(process, used);
            it->setUsed(used);
        } else {
            // El hueco es más grande, lo dividimos
            int remainingSize = it->getSize() - size;
            int startPos = it->getStart();
            
            // Crear bloque ocupado
            *it = MemoryBlock(startPos, size, process, false, used);
            
            // Insertar bloque libre con el espacio restante
            ++it;
            memory.insert(it, MemoryBlock(startPos + size, remainingSize, "", true));
        }
    }
    
public:
    
    // Liberar memoria de un proceso
    bool deallocate(string process) {
        for (auto it = memory.begin(); it != memory.end(); ++it) {
            if (!it->getIsFree() && it->getProcess() == process) {
                it->free();
                
                // Combinar con bloques libres adyacentes
                mergeAdjacentFreeBlocks();
                return true;
            }
        }
        return false; // Proceso no encontrado
    }
    
    // Combinar bloques libres adyacentes
    void mergeAdjacentFreeBlocks() {
        auto it = memory.begin();
        while (it != memory.end()) {
            auto next = it;
            ++next;
            
            if (next != memory.end() && it->getIsFree() && next->getIsFree()) {
                // Combinar bloques
                it->increaseSize(next->getSize());
                memory.erase(next);
            } else {
                ++it;
            }
        }
    }
    
    // Mostrar el estado de la memoria
    void displayMemory() {
        cout << "\n=== ESTADO DE LA MEMORIA ===" << endl;
        cout << "Memoria total: " << totalMemory << " unidades\n" << endl;
        
        cout << left << setw(10) << "Inicio" 
             << setw(10) << "Tamano" 
             << setw(15) << "Proceso" 
             << setw(10) << "Estado" << endl;
        cout << string(45, '-') << endl;
        
        for (const auto& block : memory) {
            cout << left << setw(10) << block.getStart()
                 << setw(10) << block.getSize()
                 << setw(15) << (block.getIsFree() ? "-" : block.getProcess())
                 << setw(10) << (block.getIsFree() ? "LIBRE" : "OCUPADO") << endl;
        }
        
        // Mapa visual de la memoria
        cout << "\nMapa visual:" << endl;
        cout << "[";
        for (const auto& block : memory) {
            char symbol = block.getIsFree() ? '.' : '#';
            // Ajustar la visualización para que sea proporcional
            int visualSize = (block.getSize() * 50) / totalMemory;
            if (visualSize < 1) visualSize = 1;
            
            for (int i = 0; i < visualSize; i++) {
                cout << symbol;
            }
        }
        cout << "]" << endl;
        cout << "Leyenda: # = Ocupado, . = Libre\n" << endl;
        
        // Estadísticas: calcular uso real y fragmentación
        int reservedUsed = 0;      // suma de tamanos reservados por procesos (bloques ocupados)
        int actualUsed = 0;        // suma de 'used' dentro de los bloques ocupados
        int internalFrag = 0;      // fragmentación interna = sum(reservado - usado)
        int totalFree = 0;        
        int largestFree = 0;      // mayor hueco libre

        for (const auto& block : memory) {
            if (!block.getIsFree()) {
                reservedUsed += block.getSize();
                actualUsed += block.getUsed();
                internalFrag += (block.getSize() - block.getUsed());
            } else {
                totalFree += block.getSize();
                if (block.getSize() > largestFree) largestFree = block.getSize();
            }
        }

        int externalFrag = totalFree - largestFree;
        if (externalFrag < 0) externalFrag = 0;

        cout << "Memoria reservada (ocupada por procesos): " << reservedUsed << " / " << totalMemory
             << " (" << fixed << setprecision(1) << (100.0 * reservedUsed / totalMemory) << "%)\n";
        cout << "Uso real (datos en los procesos): " << actualUsed << " / " << totalMemory
             << " (" << fixed << setprecision(1) << (100.0 * actualUsed / totalMemory) << "%)\n";
        cout << "Fragmentacion interna (FI): " << internalFrag << " unidades\n";
        cout << "Fragmentacion externa (FE): " << externalFrag << " unidades (Hueco libre maximo: " << largestFree << ")\n";
        cout << string(50, '=') << endl;
    }
    
    // Mostrar el estado de la memoria en formato compacto
    void displayMemoryCompact() {
        for (const auto& block : memory) {
            cout << "[";
            if (block.getIsFree()) {
                cout << "Libre: " << block.getSize();
            } else {
                cout << block.getProcess() << ": " << block.getSize();
            }
            cout << "]";
        }
        cout << endl;
    }
    
    // Obtener el nombre del algoritmo
    string getAlgorithmName() const {
        switch (algorithm) {
            case FIRST_FIT: return "First Fit";
            case BEST_FIT: return "Best Fit";
            case WORST_FIT: return "Worst Fit";
            default: return "Desconocido";
        }
    }
};
