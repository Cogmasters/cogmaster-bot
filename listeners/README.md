# Interaction Listeners

## Activate interaction listeners in your guild

Before you get started, make sure to locate the `cog_bot` field of your bot's `config.json`, and
update it with your guild primitives.

Once you are ready, you can come back to this folder and continue:

1. Build executables:
    ```bash
    $ make
    ```
2. (optional) Modify the default listeners by editing the sub-folder's JSON files.
3. Run executables with the relative path of the listener JSON to be activated
  * **Example:**
    ```bash
    $ ./menu rubberduck-channel/menu.json
    ```

# Listeners Listing

## [rubberduck-channel/](rubberduck-channel/)
### [`menu.json`](rubberduck-channel/menu.json) - Initialize the channel-create menu message to guild's `#rules` channel
### [`cmd.json`](rubberduck-channel/cmd.json) - Set the application commands the users may use for editing their channel

## [select-roles/](select-roles/)
### [`menu.json`](select-roles/menu.json) - Initialize the role-picking menu message to guild's `#rules` channel
