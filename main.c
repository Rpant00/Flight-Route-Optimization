#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#define INF INT_MAX

typedef struct {
    char code[4];
    char name[50];
    float latitude;
    float longitude;
} Airport;

typedef struct {
    int src;
    int dest;
    int distance;
    int duration;
    int cost;
} Route;

typedef struct {
    Airport airports[100];
    int numAirports;
    struct AdjListNode* adjList[100];
} FlightNetwork;

typedef struct AdjListNode {
    int dest;
    int distance;
    int duration;
    int cost;
    struct AdjListNode* next;
} AdjListNode;

typedef struct {
    int airport;
    int distance;
    int totalDuration;
    int totalCost;
} HeapNode;

typedef struct {
    int capacity;
    int size;
    int *pos;
    HeapNode** array;
} MinHeap;

AdjListNode* createAdjListNode(int dest, int distance, int duration, int cost) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    newNode->dest = dest;
    newNode->distance = distance;
    newNode->duration = duration;
    newNode->cost = cost;
    newNode->next = NULL;
    return newNode;
}

FlightNetwork* createFlightNetwork() {
    FlightNetwork* network = (FlightNetwork*)malloc(sizeof(FlightNetwork));
    network->numAirports = 0;
    
    for (int i = 0; i < 100; i++) {
        network->adjList[i] = NULL;
    }
    
    return network;
}

int addAirport(FlightNetwork* network, const char* code, const char* name, float latitude, float longitude) {
    if (network->numAirports >= 100) {
        printf("Error: Maximum number of airports reached\n");
        return -1;
    }
    
    for (int i = 0; i < network->numAirports; i++) {
        if (strcmp(network->airports[i].code, code) == 0) {
            printf("Airport %s already exists\n", code);
            return i;
        }
    }
    
    int index = network->numAirports;
    strcpy(network->airports[index].code, code);
    strcpy(network->airports[index].name, name);
    network->airports[index].latitude = latitude;
    network->airports[index].longitude = longitude;
    
    network->numAirports++;
    return index;
}

int findAirportIndex(FlightNetwork* network, const char* code) {
    for (int i = 0; i < network->numAirports; i++) {
        if (strcmp(network->airports[i].code, code) == 0) {
            return i;
        }
    }
    return -1;
}

void addRoute(FlightNetwork* network, const char* srcCode, const char* destCode, 
              int distance, int duration, int cost) {
    int srcIndex = findAirportIndex(network, srcCode);
    int destIndex = findAirportIndex(network, destCode);
    
    if (srcIndex == -1 || destIndex == -1) {
        printf("Error: One or both airports not found\n");
        return;
    }
    
    AdjListNode* newNode = createAdjListNode(destIndex, distance, duration, cost);
    newNode->next = network->adjList[srcIndex];
    network->adjList[srcIndex] = newNode;
}

MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->pos = (int*)malloc(capacity * sizeof(int));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (HeapNode**)malloc(capacity * sizeof(HeapNode*));
    return minHeap;
}

HeapNode* createHeapNode(int v, int dist, int duration, int cost) {
    HeapNode* node = (HeapNode*)malloc(sizeof(HeapNode));
    node->airport = v;
    node->distance = dist;
    node->totalDuration = duration;
    node->totalCost = cost;
    return node;
}

void swapHeapNodes(HeapNode** a, HeapNode** b) {
    HeapNode* temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest, left, right;
    smallest = idx;
    left = 2 * idx + 1;
    right = 2 * idx + 2;
    
    if (left < minHeap->size && 
        minHeap->array[left]->distance < minHeap->array[smallest]->distance)
        smallest = left;
    
    if (right < minHeap->size && 
        minHeap->array[right]->distance < minHeap->array[smallest]->distance)
        smallest = right;
    
    if (smallest != idx) {
        HeapNode* smallestNode = minHeap->array[smallest];
        HeapNode* idxNode = minHeap->array[idx];
        
        minHeap->pos[smallestNode->airport] = idx;
        minHeap->pos[idxNode->airport] = smallest;
        
        swapHeapNodes(&minHeap->array[smallest], &minHeap->array[idx]);
        
        minHeapify(minHeap, smallest);
    }
}

int isEmpty(MinHeap* minHeap) {
    return minHeap->size == 0;
}

HeapNode* extractMin(MinHeap* minHeap) {
    if (isEmpty(minHeap))
        return NULL;
    
    HeapNode* root = minHeap->array[0];
    
    HeapNode* lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;
    
    minHeap->pos[root->airport] = minHeap->size - 1;
    minHeap->pos[lastNode->airport] = 0;
    
    --minHeap->size;
    minHeapify(minHeap, 0);
    
    return root;
}

void decreaseKey(MinHeap* minHeap, int v, int dist, int duration, int cost) {
    int i = minHeap->pos[v];
    
    minHeap->array[i]->distance = dist;
    minHeap->array[i]->totalDuration = duration;
    minHeap->array[i]->totalCost = cost;
    
    while (i && minHeap->array[i]->distance < minHeap->array[(i - 1) / 2]->distance) {
        minHeap->pos[minHeap->array[i]->airport] = (i - 1) / 2;
        minHeap->pos[minHeap->array[(i - 1) / 2]->airport] = i;
        swapHeapNodes(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
        
        i = (i - 1) / 2;
    }
}

bool isInMinHeap(MinHeap* minHeap, int v) {
    if (minHeap->pos[v] < minHeap->size)
        return true;
    return false;
}

void printPath(int* prev, int dest, FlightNetwork* network) {
    if (prev[dest] == -1) {
        printf("%s", network->airports[dest].code);
        return;
    }
    
    printPath(prev, prev[dest], network);
    printf(" -> %s", network->airports[dest].code);
}

void dijkstra(FlightNetwork* network, int src, int dest) {
    int V = network->numAirports;
    
    int* dist = (int*)malloc(V * sizeof(int));
    int* duration = (int*)malloc(V * sizeof(int));
    int* cost = (int*)malloc(V * sizeof(int));
    
    int* prev = (int*)malloc(V * sizeof(int));
    
    MinHeap* minHeap = createMinHeap(V);
    
    for (int v = 0; v < V; v++) {
        dist[v] = INF;
        duration[v] = INF;
        cost[v] = INF;
        prev[v] = -1;
        minHeap->array[v] = createHeapNode(v, dist[v], duration[v], cost[v]);
        minHeap->pos[v] = v;
    }
    
    dist[src] = 0;
    duration[src] = 0;
    cost[src] = 0;
    decreaseKey(minHeap, src, dist[src], duration[src], cost[src]);
    
    minHeap->size = V;
    
    while (!isEmpty(minHeap)) {
        HeapNode* minNode = extractMin(minHeap);
        int u = minNode->airport;
        
        if (u == dest)
            break;
        
        AdjListNode* pCrawl = network->adjList[u];
        while (pCrawl != NULL) {
            int v = pCrawl->dest;
            
            if (isInMinHeap(minHeap, v) && dist[u] != INF && 
                pCrawl->distance + dist[u] < dist[v]) {
                
                dist[v] = dist[u] + pCrawl->distance;
                duration[v] = duration[u] + pCrawl->duration;
                cost[v] = cost[u] + pCrawl->cost;
                
                prev[v] = u;
                
                decreaseKey(minHeap, v, dist[v], duration[v], cost[v]);
            }
            pCrawl = pCrawl->next;
        }
        
        free(minNode);
    }
    
    if (dist[dest] == INF) {
        printf("No path exists from %s to %s\n", 
               network->airports[src].code, network->airports[dest].code);
    } else {
        printf("Optimal route from %s to %s:\n", 
               network->airports[src].code, network->airports[dest].code);
        printf("Path: ");
        printPath(prev, dest, network);
        printf("\nTotal Distance: %d units\n", dist[dest]);
        printf("Total Duration: %d minutes\n", duration[dest]);
        printf("Total Cost: %d units\n", cost[dest]);
    }
    
    free(dist);
    free(duration);
    free(cost);
    free(prev);
    for (int i = 0; i < minHeap->capacity; i++) {
        if (i < minHeap->size && minHeap->array[i]) {
            free(minHeap->array[i]);
        }
    }
    free(minHeap->array);
    free(minHeap->pos);
    free(minHeap);
}

void findMinCostRoute(FlightNetwork* network, int src, int dest) {
    printf("Finding minimum cost route...\n");
}

void freeNetwork(FlightNetwork* network) {
    for (int i = 0; i < network->numAirports; i++) {
        AdjListNode* current = network->adjList[i];
        while (current) {
            AdjListNode* next = current->next;
            free(current);
            current = next;
        }
    }
    
    free(network);
}

int main() {
    FlightNetwork* network = createFlightNetwork();
    
    addAirport(network, "DEL", "Indira Gandhi International Airport", 28.5561, 77.0994);
    addAirport(network, "BOM", "Chhatrapati Shivaji International Airport", 19.0896, 72.8656);
    addAirport(network, "MAA", "Chennai International Airport", 12.9941, 80.1709);
    addAirport(network, "BLR", "Kempegowda International Airport", 13.1986, 77.7066);
    addAirport(network, "HYD", "Rajiv Gandhi International Airport", 17.2403, 78.4294);
    addAirport(network, "CCU", "Netaji Subhas Chandra Bose International Airport", 22.6547, 88.4467);
    addAirport(network, "COK", "Cochin International Airport", 10.1520, 76.3922);
    
    addRoute(network, "DEL", "BOM", 1148, 125, 7500);
    addRoute(network, "DEL", "MAA", 1760, 150, 8500);
    addRoute(network, "DEL", "BLR", 1740, 150, 8200);
    addRoute(network, "DEL", "CCU", 1300, 120, 7000);
    
    addRoute(network, "BOM", "DEL", 1148, 130, 7800);
    addRoute(network, "BOM", "BLR", 845, 90, 5000);
    addRoute(network, "BOM", "HYD", 620, 70, 4500);
    
    addRoute(network, "MAA", "DEL", 1760, 155, 8700);
    addRoute(network, "MAA", "BLR", 284, 45, 3000);
    addRoute(network, "MAA", "COK", 500, 60, 3500);
    
    addRoute(network, "BLR", "DEL", 1740, 155, 8500);
    addRoute(network, "BLR", "BOM", 845, 95, 5200);
    addRoute(network, "BLR", "MAA", 284, 50, 3200);
    addRoute(network, "BLR", "HYD", 500, 60, 3800);
    
    addRoute(network, "HYD", "BOM", 620, 75, 4700);
    addRoute(network, "HYD", "BLR", 500, 65, 4000);
    
    addRoute(network, "CCU", "DEL", 1300, 125, 7200);
    addRoute(network, "CCU", "MAA", 1370, 130, 7800);
    
    addRoute(network, "COK", "MAA", 500, 65, 3700);
    addRoute(network, "COK", "BLR", 360, 55, 3500);
    
    int srcIndex = findAirportIndex(network, "DEL");
    int destIndex = findAirportIndex(network, "COK");
    
    if (srcIndex != -1 && destIndex != -1) {
        printf("\n=== Finding optimal route based on distance ===\n");
        dijkstra(network, srcIndex, destIndex);
    } else {
        printf("Invalid source or destination airport\n");
    }
    
    freeNetwork(network);
    
    return 0;
}
