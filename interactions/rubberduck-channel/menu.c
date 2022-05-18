#include <stdlib.h>
#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <concord/discord.h>

#include "interactions.h"

/** @brief Per-request context storage for async functions */
struct context {
    /** the current interaction */
    const struct discord_interaction *interaction;
    /** whether the channel is private or public */
    bool priv;
};

static void
context_cleanup(struct discord *cogbot, void *p_cxt)
{
    struct context *cxt = p_cxt;
    discord_unclaim(cogbot, cxt->interaction);
    free(cxt);
}

static void
done_create_channel(struct discord *cogbot,
                    struct discord_response *resp,
                    const struct discord_channel *channel)
{
    struct context *cxt = resp->data;

    char welcome_msg[DISCORD_MAX_MESSAGE_LEN];

    snprintf(
        welcome_msg, sizeof(welcome_msg),
        "Welcome <@!%" PRIu64 ">, "
        "I hope you enjoy your new space! Check your available commands by "
        "typing `/mychannel` for further channel customization!",
        cxt->interaction->member->user->id);

    discord_create_message(cogbot, channel->id,
                           &(struct discord_create_message){
                               .content = welcome_msg,
                           },
                           NULL);

    discord_edit_original_interaction_response(
        cogbot, cxt->interaction->application_id, cxt->interaction->token,
        &(struct discord_edit_original_interaction_response){
            .content = "Your channel has been created!",
        },
        NULL);
}

static void
fail_create_channel(struct discord *cogbot, struct discord_response *resp)
{
    struct context *cxt = resp->data;

    discord_edit_original_interaction_response(
        cogbot, cxt->interaction->application_id, cxt->interaction->token,
        &(struct discord_edit_original_interaction_response){
            .content = "Couldn't create channel, please report to our staff.",
        },
        NULL);
}

static void
done_role_add(struct discord *cogbot, struct discord_response *resp)
{
    struct cogbot_primitives *primitives = discord_get_data(cogbot);
    struct context *cxt = resp->data;

    struct discord_overwrite overwrites[] = {
        /* give read/write permission to user */
        {
            .id = cxt->interaction->member->user->id,
            .type = 1,
            .allow = PERMS_READ | PERMS_WRITE,
        },
        /* give read/write permission for @helper */
        {
            .id = primitives->roles.helper_id,
            .type = 0,
            .allow = PERMS_READ | PERMS_WRITE,
        },
        /* hide it from @watcher only if 'priv' has been set */
        {
            .id = primitives->roles.watcher_id,
            .type = 0,
            .allow = cxt->priv ? 0 : PERMS_READ,
        },
        /* give write permissions to @everyone (not read) */
        {
            .id = primitives->guild_id,
            .type = 0,
            .deny = PERMS_READ,
            .allow = PERMS_WRITE,
        },
    };

    /* create user channel */
    discord_create_guild_channel(
        cogbot, primitives->guild_id,
        &(struct discord_create_guild_channel){
            .name = cxt->interaction->member->user->username,
            .permission_overwrites =
                &(struct discord_overwrites){
                    .size = sizeof(overwrites) / sizeof *overwrites,
                    .array = overwrites,
                },
            .parent_id = primitives->category_id,
        },
        &(struct discord_ret_channel){
            .done = done_create_channel,
            .fail = fail_create_channel,
            .data = cxt,
        });
}

static void
fail_role_add(struct discord *cogbot, struct discord_response *resp)
{
    struct cogbot_primitives *primitives = discord_get_data(cogbot);
    struct context *cxt = resp->data;

    char diagnosis[256];

    snprintf(diagnosis, sizeof(diagnosis),
             "Couldn't assign role <@&%" PRIu64
             ">, please report to our staff.",
             primitives->roles.rubberduck_id);

    discord_edit_original_interaction_response(
        cogbot, cxt->interaction->application_id, cxt->interaction->token,
        &(struct discord_edit_original_interaction_response){
            .content = diagnosis,
        },
        NULL);
}

void
react_rubberduck_channel_menu(struct discord *cogbot,
                              struct discord_interaction_response *params,
                              const struct discord_interaction *interaction)
{
    struct cogbot_primitives *primitives = discord_get_data(cogbot);
    bool priv = false;

    /* skip user with already assigned channel */
    if (is_included_role(interaction->member->roles,
                         primitives->roles.rubberduck_id))
    {
        params->data->content =
            "It seems you already have a channel, please edit it from within";
        return;
    }

    /* get channel visibility */
    if (interaction->data->values)
        for (int i = 0; i < interaction->data->values->size; ++i) {
            char *value = interaction->data->values->array[i];

            if (0 == strcmp(value, "private")) priv = true;
        }

    struct context *cxt = malloc(sizeof *cxt);
    *cxt = (struct context){
        .interaction = discord_claim(cogbot, interaction),
        .priv = priv,
    };

    /* assign channel owner role to user */
    discord_add_guild_member_role(cogbot, primitives->guild_id,
                                  interaction->member->user->id,
                                  primitives->roles.rubberduck_id,
                                  &(struct discord_ret){
                                      .done = &done_role_add,
                                      .fail = &fail_role_add,
                                      .data = cxt,
                                      .cleanup = &context_cleanup,
                                  });

    params->type = DISCORD_INTERACTION_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
