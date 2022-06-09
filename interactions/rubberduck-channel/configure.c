#include <stdlib.h>
#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <concord/discord.h>

#include "interactions.h"

/** @brief Per-request context storage for async functions */
struct context {
    /** user's rubberduck channel */
    u64snowflake channel_id;
    /** whether rubberduck channel is private */
    bool priv;
};

static void
context_cleanup(struct discord *cogbot, void *p_cxt)
{
    (void)cogbot;
    free(p_cxt);
}

static void
done_edit_permissions(struct discord *cogbot, struct discord_response *resp)
{
    const struct discord_interaction *interaction = resp->keep;
    struct context *cxt = resp->data;
    char diagnosis[256];

    snprintf(diagnosis, sizeof(diagnosis),
             "Completed action targeted to <#%" PRIu64 ">.", cxt->channel_id);

    discord_edit_original_interaction_response(
        cogbot, interaction->application_id, interaction->token,
        &(struct discord_edit_original_interaction_response){
            .content = diagnosis,
        },
        NULL);
}

static void
fail_edit_permissions(struct discord *cogbot, struct discord_response *resp)
{
    const struct discord_interaction *interaction = resp->keep;
    struct context *cxt = resp->data;
    char diagnosis[256];

    snprintf(diagnosis, sizeof(diagnosis),
             "Failed action targeted to <#%" PRIu64 ">.", cxt->channel_id);

    discord_edit_original_interaction_response(
        cogbot, interaction->application_id, interaction->token,
        &(struct discord_edit_original_interaction_response){
            .content = diagnosis,
        },
        NULL);
}

static void
done_get_channel(struct discord *cogbot,
                 struct discord_response *resp,
                 const struct discord_channel *channel)
{
    struct cogbot_primitives *primitives = discord_get_data(cogbot);
    const struct discord_interaction *interaction = resp->keep;

    if (!is_user_rubberduck_channel(channel, primitives->category_id,
                                    interaction->member->user->id))
    {
        discord_edit_original_interaction_response(
            cogbot, interaction->application_id, interaction->token,
            &(struct discord_edit_original_interaction_response){
                .content = "Couldn't complete operation. Make sure to use "
                           "`/mycommand` from your channel" },
            NULL);
    }
    else {
        struct context *cxt = resp->data;
        cxt->channel_id = channel->id;

        /* edit user channel */
        discord_edit_channel_permissions(
            cogbot, channel->id, primitives->roles.watcher_id,
            &(struct discord_edit_channel_permissions){
                .type = 0,
                .allow = cxt->priv ? 0 : PERMS_READ,
            },
            &(struct discord_ret){
                .done = &done_edit_permissions,
                .fail = &fail_edit_permissions,
                .keep = interaction,
                .data = cxt,
                .cleanup = &context_cleanup,
            });
    }
}

void
react_rubberduck_channel_configure(
    struct discord *cogbot,
    struct discord_interaction_response *params,
    const struct discord_interaction *interaction,
    struct discord_application_command_interaction_data_options *options)
{
    char *visibility = NULL;

    if (options)
        for (int i = 0; i < options->size; ++i) {
            char *name = options->array[i].name;
            char *value = options->array[i].value;

            if (0 == strcmp(name, "visibility")) visibility = value;
        }

    if (!visibility) {
        params->data->content = "No operation will be taking place.";
        return;
    }

    struct context *cxt = malloc(sizeof *cxt);
    *cxt = (struct context){
        .priv = (0 == strcmp(visibility, "private")),
    };

    discord_get_channel(cogbot, interaction->channel_id,
                        &(struct discord_ret_channel){
                            .done = &done_get_channel,
                            .fail = &fail_edit_permissions,
                            .keep = interaction,
                            .data = cxt,
                            .cleanup = &context_cleanup,
                        });

    params->type = DISCORD_INTERACTION_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
