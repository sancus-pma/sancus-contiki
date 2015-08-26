#include "connections.h"

#include <stdlib.h>

typedef struct Node
{
    Connection connection;
    struct Node* next;
} Node;

static Node* connections_head = NULL;

int connections_add(Connection* connection)
{
    Node* node = malloc(sizeof(Node));

    if (node == NULL)
        return 0;

    node->connection = *connection;
    node->next = connections_head;
    connections_head = node;
    return 1;
}

Connection* connections_get(sm_id from_sm, io_index from_output)
{
    Node* current = connections_head;

    while (current != NULL)
    {
        Connection* connection = &current->connection;

        if (connection->from_sm == from_sm &&
            connection->from_output == from_output)
        {
            return connection;
        }

        current = current->next;
    }

    return NULL;
}
