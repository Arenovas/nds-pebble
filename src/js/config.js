module.exports = [
    {
        "type": "heading",
        "defaultValue": "App Configuration"
    },
/*{
    "type": "text",
    "defaultValue": "Here is some introductory text."
},
{
    "type": "section",
    "items": [
        {
            "type": "heading",
            "defaultValue": "Colors"
        },
        {
            "type": "color",
            "messageKey": "BackgroundColor",
            "defaultValue": "0x000000",
            "label": "Background Color"
        },
        {
            "type": "color",
            "messageKey": "ForegroundColor",
            "defaultValue": "0xFFFFFF",
            "label": "Foreground Color"
        }
    ]
},*/
{
    "type": "section",
    "items": [
        /*{
            "type": "heading",
            "defaultValue": "More Settings"
        },*/
        {
            "type": "toggle",
            "messageKey": "SecondTick",
            "label": "Enable Seconds",
            "defaultValue": true
        /*},
        {
            "type": "toggle",
            "messageKey": "Animations",
            "label": "Enable Animations",
            "defaultValue": false*/
        },
        {
            "type": "select",
            "messageKey": "FavColor",
            "defaultValue": "0",
            "label": "Favorite Color",
            "options": [
                {
                    "label": "Gray",
                    "value": "0"
                },
                {
                    "label": "Brown",
                    "value": "1"
                },
                {
                    "label": "Red",
                    "value": "2"
                },
                {
                    "label": "Pink",
                    "value": "3"
                },
                {
                    "label": "Orange",
                    "value": "4"
                },
                {
                    "label": "Yellow",
                    "value": "5"
                },
                {
                    "label": "Yellow-Green",
                    "value": "6"
                },
                {
                    "label": "Green",
                    "value": "7"
                },
                {
                    "label": "Dark Green",
                    "value": "8"
                },
                {
                    "label": "Seafoam",
                    "value": "9"
                },
                {
                    "label": "Light Blue",
                    "value": "10"
                },
                {
                    "label": "Blue",
                    "value": "11"
                },
                {
                    "label": "Dark Blue",
                    "value": "12"
                },
                {
                    "label": "Purple",
                    "value": "13"
                },
                {
                    "label": "Light Purple",
                    "value": "14"
                },
                {
                    "label": "Magenta",
                    "value": "15"
                }
            ]
        }
    ]
},
{
    "type": "submit",
    "defaultValue": "Save Settings"
}
];
