#ifndef INTERACTIONS_H
#define INTERACTIONS_H

/* see https://discordapi.com/permissions.html#1024 */
#define PERMS_READ (1024)
/* see https://discordapi.com/permissions.html#448824526912 */
#define PERMS_WRITE (448824526912)
/* all bits set */
#define PERMS_ALL ((enum discord_bitwise_permission_flags) - 1)

/** @brief The client environment to work with */
struct cogbot_primitives {
    /** the guild our client will react to */
    u64snowflake guild_id;
    /** the rubberduck channels category id */
    u64snowflake category_id;
    struct {
        /** role for users that own a rubberduck channel */
        u64snowflake rubberduck_id;
        /**
         * role for users that have read/write access to public and private
         *        rubberduck channels
         */
        u64snowflake helper_id;
        /** role for users that want to make public rubberduck channels visible
         */
        u64snowflake watcher_id;
        /** role for users that want to subscribe to guild announcements */
        u64snowflake announcements_id;
        /** role for users that are Linux users */
        u64snowflake linux_id;
        /** role for users that are Windows users */
        u64snowflake windows_id;
        /** role for users that are macOS users */
        u64snowflake macos_id;
    } roles;
};

/**
 * @brief React to rubberduck channel selection menu
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 */
void react_rubberduck_channel_menu(
    struct discord *client,
    struct discord_interaction_response *params,
    const struct discord_interaction *interaction);

/**
 * @brief React to channel subscriptions selection menu
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 */
void react_select_subscriptions_menu(
    struct discord *cogbot,
    struct discord_interaction_response *params,
    const struct discord_interaction *interaction);

/**
 * @brief React to OS selection menu
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 */
void react_select_OS(struct discord *cogbot,
                     struct discord_interaction_response *params,
                     const struct discord_interaction *interaction);

/**
 * @brief React to rubberduck channel 'action' command
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 * @param options the options selected by user
 */
void react_rubberduck_channel_action(
    struct discord *cogbot,
    struct discord_interaction_response *params,
    const struct discord_interaction *interaction,
    struct discord_application_command_interaction_data_options *options);

/**
 * @brief React to rubberduck channel 'delete' command
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 * @param options the options selected by user
 */
void react_rubberduck_channel_delete(
    struct discord *client,
    struct discord_interaction_response *params,
    const struct discord_interaction *interaction,
    struct discord_application_command_interaction_data_options *options);

/**
 * @brief React to rubberduck channel configure
 *
 * @param client the bot client
 * @param params the interaction response to be sent at `main.c`
 * @param interaction the interaction object received
 * @param options the options selected by user
 */
void react_rubberduck_channel_configure(
    struct discord *client,
    struct discord_interaction_response *params,
    const struct discord_interaction *interaction,
    struct discord_application_command_interaction_data_options *options);

/******************************************************************************
 * Utility functions
 ******************************************************************************/

/**
 * @brief Check if is a rubber duck channel owned by user
 *
 * @param the channel to be checked against
 * @param rubberduck_category_id the rubberduck channels category id
 * @param user_id user to be checked for ownership
 * @return `true` if channel belongs to user
 */
bool is_user_rubberduck_channel(const struct discord_channel *channel,
                                u64snowflake rubberduck_category_id,
                                u64snowflake user_id);

/**
 * @brief Check if role is included in list
 *
 * @param roles the role ids list to be checked against
 * @param role_id the expected role id
 * @return `true` if role_id is included in roles list
 */
bool is_included_role(struct snowflakes *roles, u64snowflake role_id);

#endif /* INTERACTIONS_H */
