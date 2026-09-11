import re
from mkdocs.exceptions import PluginError

def on_post_page(output, page, config):
    """
    Verifies that pages containing ':::' directives successfully 
    generate mkdocstrings API documentation elements in the final HTML.
    This verification is important because mkdocs can silently fail to generate the API docs.
    """
    # Ensure the source markdown exists and contains the mkdocstrings syntax

    if not page.markdown and re.search(r"^\s*:::\s+", page.markdown, re.MULTILINE):
        # Must contain either function or class definitions
        has_docstrings_output = '<nav class="md-nav" aria-label="Classes">' in output or '<nav class="md-nav" aria-label="Functions:">' in output
        
        if not has_docstrings_output:
            raise PluginError(
                f"Build Error: Page '{page.file.src_path}' contains ':::' directives, "
                f"but no mkdocstrings output was found in the generated HTML. "
                f"Check if the python module path is correct or if mkdocstrings failed to process it."
            )
            
    return output