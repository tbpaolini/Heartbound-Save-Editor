/* Template and data structure for the saved data of Heartbound */

// Player attributes
char *seed_code[10];            // Game seed (10 decimal characters long)
char *room_id[30];              // The ID (as a string) of the room the player is
double x_axis, y_axis;          // Coordinates of the player in the room
double hp_current, hp_maximum;  // Current and maximum hit points of the player

// Which glyph sets the player know:
// 0 = None; 1 = Lightbringer; 2 = Lightbringer and Darksider
char known_glyphs;

// The storyline variables
struct StorylineVars
{
    unsigned int value;
    char name[30];
    char info[100];
    char detail[50];
    char max;
    char **aliases;
} save_data[1000];