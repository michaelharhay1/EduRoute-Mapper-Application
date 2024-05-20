
/* 
 * Authors: Michael Harhay, Phoebe Owusu, Vanessa Poiana
 * 
 * Date: 26/02/2024
 *
 * Description: contains global variables and callback functions definitions. Includes variables with info
 * about visible world, search bar functionality, POI buttons, game mode, and search bar auto-complete functionality.
 * Callback function definitions are for drawing new map, initializing GUI components, handling user interactions, etc.
*/

#include "callbacks.h"
#include "m3.h"
#include "graphics_data/city_information.h"
#include "graphics_data/graphics_globals.h"

#include <limits.h>
#include <thread>

//---------------------------------------- Search Callback Functions -----------------------------------------//
// Callback function for when the return key is pressed while the user is typing, will input the first valid
// street name into the search bar
void act_on_search_match_selected(GtkEntry *entry, gpointer /*user_data*/, ezgl::application * /*application*/)
{

    // Get Entry that was edited
    const gchar *entry_text = gtk_entry_get_text(entry);

    // Write name to entry box
    std::vector<StreetIdx> street_ids = findStreetIdsFromPartialStreetName(entry_text);

    // Write it into the entry
    if (street_ids.size() > 0)
    {
        gtk_entry_set_text(entry, Streets_graphics_data->all_streets[street_ids[0]].name.c_str());
    }
}

// Callback for when the user clicks on one of the matches, will input the string selected from the dropdown
gboolean act_on_search_match_completion_selected(GtkEntryCompletion *widget, GtkTreeModel *model, GtkTreeIter *iter_in, gpointer /*user_data*/)
{

    GtkEntry *entry = GTK_ENTRY(gtk_entry_completion_get_entry(widget));
    gchar *selected_text = NULL;

    // Get match text
    if (iter_in != NULL)
    {
        gtk_tree_model_get(model, iter_in, 0, &selected_text, -1);
        std::string selected_text_str(selected_text);

        if (selected_text != NULL)
        {
            gtk_entry_set_text(entry, selected_text);
            g_free(selected_text);
        }
    }

    return true;
}
//-------------------------------------- End Draw Callback Functions ---------------------------------------//
