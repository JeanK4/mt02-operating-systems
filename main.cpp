#include "memory_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

void processFromFile(MemoryManager& mm, const string& filename, bool compactMode=false) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return;
    }

    string line;
    int lineNumber = 0;
    while (getline(file, line)) {
        lineNumber++;
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        char command;
        iss >> command;

        switch (command) {
            case 'A': {
                string process; int size;
                iss >> process >> size;
                int used;
                bool hasUsed = false;
                if (iss >> used) hasUsed = true;

                if (!compactMode) {
                    cout << "Comando: A " << process << " " << size;
                    if (hasUsed) cout << " " << used;
                    cout << endl;
                }

                bool ok = false;
                if (hasUsed) ok = mm.allocate(process, size, used);
                else ok = mm.allocate(process, size);

                if (ok) {
                    if (!compactMode) cout << "Asignacion exitosa" << endl;
                } else {
                    cout << "Error: No hay espacio suficiente para " << process << endl;
                }
                break;
            }
            case 'L': {
                string process; iss >> process;
                if (!compactMode) cout << "Comando: L " << process << endl;
                if (mm.deallocate(process)) {
                    if (!compactMode) cout << "Memoria liberada" << endl;
                } else {
                    cout << "Error: Proceso " << process << " no encontrado" << endl;
                }
                break;
            }
            case 'M': {
                if (compactMode) mm.displayMemoryCompact(); else mm.displayMemory();
                break;
            }
            default:
                cerr << "Advertencia (linea " << lineNumber << "): Comando desconocido '" << command << "'" << endl;
        }
    }

    file.close();
}

void processFromConsole(MemoryManager& mm) {
    cout << "\n=== MODO INTERACTIVO ===" << endl;
    cout << "Comandos: A <proc> <tam>, L <proc>, M, Q" << endl;
    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        istringstream iss(line);
        char command; iss >> command; command = toupper(command);
        switch (command) {
            case 'A': {
                string process; int size; iss >> process >> size;
                int used; bool hasUsed = false;
                if (iss >> used) hasUsed = true;
                bool ok = hasUsed ? mm.allocate(process, size, used) : mm.allocate(process, size);
                if (ok) cout << "Asignacion exitosa" << endl; else cout << "Error: No hay espacio suficiente" << endl;
                break;
            }
            case 'L': {
                string process; iss >> process; if (mm.deallocate(process)) cout << "Memoria liberada" << endl; else cout << "Error: Proceso no encontrado" << endl; break;
            }
            case 'M': mm.displayMemory(); break;
            case 'Q': cout << "Saliendo..." << endl; return;
            default: cout << "Comando desconocido" << endl;
        }
    }
}

int main() {
    cout << "=== SIMULADOR DE GESTION DE MEMORIA (interactivo) ===" << endl;

    int memorySize = 100;
    cout << "Ingrese el tamaño de la memoria (por defecto 100): ";
    string input; getline(cin, input);
    if (!input.empty()) memorySize = stoi(input);

    cout << "\nSeleccione el algoritmo de asignacion:" << endl;
    cout << "1. First Fit" << endl;
    cout << "2. Best Fit" << endl;
    cout << "3. Worst Fit" << endl;
    cout << "Opcion (1-3): ";
    int op = 1; cin >> op; cin.ignore();
    AllocationAlgorithm algo = FIRST_FIT;
    if (op == 2) algo = BEST_FIT; else if (op == 3) algo = WORST_FIT;

    MemoryManager mm(memorySize, algo);

    cout << "\nComo desea ingresar los comandos?\n1. Desde archivo\n2. Desde consola" << endl;
    cout << "Opcion: "; int mode = 2; cin >> mode; cin.ignore();
    if (mode == 1) {
        cout << "Ingrese el nombre del archivo: "; string filename; getline(cin, filename);
        processFromFile(mm, filename, false);
    } else {
        processFromConsole(mm);
    }

    return 0;
}
