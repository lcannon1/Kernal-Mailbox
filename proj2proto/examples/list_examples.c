
/*
    list_examples.c -- Examples on how to use the linked list

    Copyright (C) 2018, 2021 Ioannis Boutsikas

    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include "list.h"


typedef struct component {
    int id;

    // This is used to link components together in their list
    struct list_head i_link_components_together;
} component_t;

typedef struct player {
    int x;
    int y;


    // This is used to link players together in the players list
    struct list_head i_link_players_together;

    // This is used so each player can have their own list of components
    struct list_head components;
} player_t;

// similar to std::vector<player_t> players in C++
// NOTE: That the list is not type-safe. e.g It does not accept
// only player_t structs
LIST_HEAD(players);

int main(void) {
    // Create the players
    int i;
    for (i = 0; i < 10; ++i) {
        player_t* new_player = (player_t*)malloc(sizeof(player_t));
        new_player->x = i;
        new_player->y = i;
        list_add_tail(&new_player->i_link_players_together, &players);
    }


    // Print the players
    {
        puts("===== Players before delete =====");
        struct  list_head *pos;
        int counter = 0;

        list_for_each(pos, &players) {
            // In here pos will be the list_head item in the corresponding player struct
            player_t* p = NULL;

            // Extract get a reference to the player_t for this iteration
            p = list_entry(pos, player_t, i_link_players_together);
            printf("Player %d {%d, %d}\n", counter, p->x, p->y);
            counter++;
        }
    }

    // Delete specific player
    {
        struct list_head* pos = NULL;
        struct list_head* tmp;

        /*
           When modifying the structure of a list (i.e removing an item)
           use the _safe variant of the function. If you do not use the _safe
           versions you run the risk of losing part of your list.
           The _safe variants require an extra variable to temporary store
           some state for the list so you don't lose any of it.
        */
        list_for_each_safe(pos, tmp, &players) {
            player_t* p = list_entry(pos, player_t, i_link_players_together);

            if (p->x == 5 && p->y == 5) {
                /*
                   ALWAYS delete the item from the list first, then free it.
                   Not the other way around.
                */
                // pos has been modified here. If you need the original node, use tmp
                list_del(pos);
                free(p);
            }
        }
    }


    {
        puts("===== Players after delete =====");

        player_t* p = NULL;
        int counter = 0;

        // Let's use a different iteration function this time.
        // There is a use case for every iteration function, just
        // use the one that fits you best.
        list_for_each_entry(p, &players, i_link_players_together) {
            printf("Player %d {%d, %d}\n", counter, p->x, p->y);
            counter++;
        }
    }

    // Create "Component List" a component in this case is the weapons
    // the player has, or the powerups, or something. Basically it is something
    // that the player "owns", and it responsible for.
    {
        struct  list_head *pos;
        list_for_each(pos, &players) {
            player_t* p = NULL;
            p = list_entry(pos, player_t, i_link_players_together);
            // Create a list at THIS player's (p) components variable
            INIT_LIST_HEAD(&p->components);
            int i;
            for (i = 0; i < 5; ++i) {
                component_t* comp = (component_t*)malloc(sizeof(component_t));
                comp->id = i;
                // Add the new component to this player's list
                list_add_tail(&comp->i_link_components_together, &p->components);
            }
        }
    }

    puts("===== Players with components  =====");
    // Print players and components
    // Here we use one iteration function for players and another for components.
    // You can mix and match depending on your needs!
    {
        struct  list_head *pos;
        int counter = 0;
        list_for_each(pos, &players) {
            player_t* p = NULL;
            p = list_entry(pos, player_t, i_link_players_together);
            printf("Player %d {%d, %d}\n", counter, p->x, p->y);
            component_t* the_component;
            /*
               A different way to iterate over the list. This gives us
               the item itself directly instead of having to call list_entry
               ourselves
            */
            list_for_each_entry(the_component, &p->components, i_link_components_together) {
                printf("\t Component id: %d\n", the_component->id);
            }
            counter++;
        }
    }

    /**
     * Let's clean up after ourselves. For every player we will clean up their components,
     * then finally clean up the player itself. Remeber _safe functions when removing!
     */
    {
        player_t* the_player;
        player_t* temp_player;
        list_for_each_entry_safe(the_player, temp_player, &players, i_link_players_together) {
            // Loop over the player's components
            component_t* the_component;
            component_t* temp_component;

            list_for_each_entry_safe(the_component, temp_component, &the_player->components, i_link_components_together) {
                // Remove the component from the list
                list_del(&the_component->i_link_components_together);
                // Free the memory
                free(the_component);
            }

            // Remove the player from the list
            list_del(&the_player->i_link_players_together);
            // Free the memory
            free(the_player);
        }
    }

    return 0;
}