#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <concord/discord.h>

#include "interactions.h"

struct discord *cogbot;

void
on_interaction_create(struct discord *cogbot,
                      const struct discord_interaction *interaction)
{
    /* Return in case of missing user input */
    if (!interaction->data) return;

    /* initialize interaction response with some default values */
    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE,
        .data =
            &(struct discord_interaction_callback_data){
                .flags = DISCORD_INTERACTION_CALLBACK_DATA_EPHEMERAL,
                .content = "⚠️ Internal Error! Interaction is "
                           "malfunctioning, please "
                           "report to the staff.",
            },
    };

    switch (interaction->type) {
    case DISCORD_INTERACTION_APPLICATION_COMMAND:
        if (0 == strcmp(interaction->data->name, "mychannel")) {

            if (interaction->data->options)
                for (int i = 0; interaction->data->options[i]; ++i) {
                    char *cmd = interaction->data->options[i]->name;

                    if (0 == strcmp(cmd, "action")) {
                        react_rubberduck_channel_action(
                            cogbot, &params, interaction,
                            interaction->data->options[i]->options);
                    }
                    else if (0 == strcmp(cmd, "configure")) {
                        react_rubberduck_channel_configure(
                            cogbot, &params, interaction,
                            interaction->data->options[i]->options);
                    }
                    else if (0 == strcmp(cmd, "delete")) {
                        react_rubberduck_channel_delete(
                            cogbot, &params, interaction,
                            interaction->data->options[i]->options);
                    }
                }
        }
        break;
    case DISCORD_INTERACTION_MESSAGE_COMPONENT:
        if (0 == strcmp(interaction->data->custom_id, "create-channel"))
            react_rubberduck_channel_menu(cogbot, &params, interaction);
        else if (0
                 == strcmp(interaction->data->custom_id,
                           "channel-subscriptions"))
            react_select_subscriptions_menu(cogbot, &params, interaction);
        else if (0 == strcmp(interaction->data->custom_id, "skill-level"))
            react_select_skill_level(cogbot, &params, interaction);
        break;
    default:
        log_error("%s (%d) is not dealt with",
                  discord_interaction_types_print(interaction->type),
                  interaction->type);
        break;
    }

    discord_create_interaction_response(cogbot, interaction->id,
                                        interaction->token, &params, NULL);
}

/* shutdown gracefully on SIGINT received */
void
sigint_handler(int signum)
{
    (void)signum;
    log_info("SIGINT received, shutting down ...");
    discord_shutdown(cogbot);
}

void
on_ready(struct discord *cogbot)
{
    const struct discord_user *bot = discord_get_self(cogbot);

    log_info("Cog-Bot succesfully connected to Discord as %s#%s!",
             bot->username, bot->discriminator);
}

struct cogbot_primitives
cogbot_get_primitives(struct discord *cogbot)
{
    struct cogbot_primitives primitives = { 0 };
    struct sized_buffer json;
    struct logconf *conf = discord_get_logconf(cogbot);

    /* get guild id */
    json = logconf_get_field(conf, "cog_bot.guild_id");
    primitives.guild_id = strtoull(json.start, NULL, 10);

    /* get rubberduck channels category id */
    json = logconf_get_field(conf, "cog_bot.category_id");
    primitives.category_id = strtoull(json.start, NULL, 10);

    /* get roles */
    json = logconf_get_field(conf, "cog_bot.roles.rubberduck_id");
    primitives.roles.rubberduck_id = strtoull(json.start, NULL, 10);
    json = logconf_get_field(conf, "cog_bot.roles.helper_id");
    primitives.roles.helper_id = strtoull(json.start, NULL, 10);
    json = logconf_get_field(conf, "cog_bot.roles.watcher_id");
    primitives.roles.watcher_id = strtoull(json.start, NULL, 10);
    json = logconf_get_field(conf, "cog_bot.roles.announcements_id");
    primitives.roles.announcements_id = strtoull(json.start, NULL, 10);
    json = logconf_get_field(conf, "cog_bot.roles.beginner_id");
    primitives.roles.beginner_id = strtoull(json.start, NULL, 10);

    return primitives;
}

int
main(int argc, char *argv[])
{
    struct cogbot_primitives primitives;

    signal(SIGINT, &sigint_handler);
    ccord_global_init();

    cogbot = discord_config_init((argc > 1) ? argv[1] : "config.json");
    assert(NULL != cogbot && "Couldn't initialize client");

    primitives = cogbot_get_primitives(cogbot);
    discord_set_data(cogbot, &primitives);

    discord_set_on_ready(cogbot, &on_ready);
    discord_set_on_interaction_create(cogbot, &on_interaction_create);

    discord_run(cogbot);

    discord_cleanup(cogbot);
    ccord_global_cleanup();
}
