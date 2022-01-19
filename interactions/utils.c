#include <concord/discord.h>

#include "interactions.h"

bool
is_user_rubberduck_channel(const struct discord_channel *channel,
                           u64_snowflake_t rubberduck_category_id,
                           u64_snowflake_t user_id)
{
    if (channel->permission_overwrites)
        for (int i = 0; channel->permission_overwrites[i]; ++i)
            if (user_id == channel->permission_overwrites[i]->id
                && rubberduck_category_id == channel->parent_id)
            {
                return true;
            }

    return false;
}

bool
is_included_role(ja_u64 **roles, u64_snowflake_t role_id)
{
    if (roles)
        for (int i = 0; roles[i]; ++i)
            if (roles[i]->value == role_id) return true;

    return false;
}
