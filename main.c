#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <concord/discord.h>

#include "interactions.h"

void
on_interaction_create(struct discord *cogbot,
                      const struct discord_interaction *interaction)
{
    /* Return in case of missing user input */
    if (!interaction->data) return;

    /* initialize interaction response with some default values */
    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data =
            &(struct discord_interaction_callback_data){
                .flags = DISCORD_MESSAGE_EPHEMERAL,
                .content = "⚠️ Internal Error! Interaction is "
                           "malfunctioning, please "
                           "report to the staff.",
            },
    };

    switch (interaction->type) {
    case DISCORD_INTERACTION_APPLICATION_COMMAND:
        if (0 == strcmp(interaction->data->name, "mychannel")) {

            if (interaction->data->options)
                for (int i = 0; i < interaction->data->options->size; ++i) {
                    char *cmd = interaction->data->options->array[i].name;

                    if (0 == strcmp(cmd, "action")) {
                        react_rubberduck_channel_action(
                            cogbot, &params, interaction,
                            interaction->data->options->array[i].options);
                    }
                    else if (0 == strcmp(cmd, "configure")) {
                        react_rubberduck_channel_configure(
                            cogbot, &params, interaction,
                            interaction->data->options->array[i].options);
                    }
                    else if (0 == strcmp(cmd, "delete")) {
                        react_rubberduck_channel_delete(
                            cogbot, &params, interaction,
                            interaction->data->options->array[i].options);
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
        else if (0 == strcmp(interaction->data->custom_id, "os"))
            react_select_OS(cogbot, &params, interaction);
        break;
    default:
        log_error("Interaction (%d code) is not dealt with",
                  interaction->type);
        break;
    }

    discord_create_interaction_response(cogbot, interaction->id,
                                        interaction->token, &params, NULL);
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
    char *path[3] = { "cog_bot", "", "" };

    /* cog_bot.guild_id */
    path[1] = "guild_id";
    json = logconf_get_field(conf, path, 2);
    primitives.guild_id = strtoull(json.start, NULL, 10);

    /* cog_bot.category_id */
    path[1] = "category_id";
    json = logconf_get_field(conf, path, 2);
    primitives.category_id = strtoull(json.start, NULL, 10);

    /* cog_bot.roles.rubberduck_id */
    path[1] = "roles";
    path[2] = "rubberduck_id";
    json = logconf_get_field(conf, path, 3);
    primitives.roles.rubberduck_id = strtoull(json.start, NULL, 10);
    /* cog_bot.roles.helper_id */
    path[2] = "helper_id";
    json = logconf_get_field(conf, path, 3);
    primitives.roles.helper_id = strtoull(json.start, NULL, 10);
    /* cog_bot.roles.watcher_id */
    path[2] = "watcher_id";
    json = logconf_get_field(conf, path, 3);
    primitives.roles.watcher_id = strtoull(json.start, NULL, 10);
    /* cog_bot.roles.announcements_id */
    path[2] = "announcements_id";
    json = logconf_get_field(conf, path, 3);
    primitives.roles.announcements_id = strtoull(json.start, NULL, 10);
    /* cog_bot.roles.linux_id */
    path[2] = "linux_id";
    json = logconf_get_field(conf, path, 3);
    primitives.roles.linux_id = strtoull(json.start, NULL, 10);
    /* cog_bot.roles.windows_id */
    path[2] = "windows_id";
    json = logconf_get_field(conf, path, 3);
    primitives.roles.windows_id = strtoull(json.start, NULL, 10);
    /* cog_bot.roles.macos_id */
    path[2] = "macos_id";
    json = logconf_get_field(conf, path, 3);
    primitives.roles.macos_id = strtoull(json.start, NULL, 10);

    return primitives;
}

int
main(int argc, char *argv[])
{
    struct cogbot_primitives primitives;
    struct discord *cogbot;

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
